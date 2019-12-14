#include "index.h"
#include "sslkeys.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <map>
#include <memory>
#include <microhttpd.h>
#include <pthread.h>
#include <sstream>
#include <unistd.h>
#include <vector>

#define DATASETSIZE (1024 * 1024)
#define POSTBUFFERSIZE 65536

using namespace std;

// Container for embedded content that shall be
// loaded and persist in memory during the server lifetime.
unique_ptr<map<string, vector<unsigned char>*> > www_data;

// Container for embedded content MIME.
unique_ptr<map<string, string> > www_data_mime;

static Index indexPage;

class Post
{
	const char* empty;
	vector<char> dataset;

public :

	Post() : empty("") { }

	const char* getDataSet()
	{
		if (dataset.size())
			return (const char*)&dataset[0];
		
		return empty;
	}

	MHD_PostProcessor* processor;

	static int process(void* con_cls, enum MHD_ValueKind kind, const char* key,
		const char* filename, const char* content_type, const char* transfer_encoding,
		const char* data, uint64_t offset, size_t size)
	{
		if (!strcmp(key, "dataset") && size)
		{
			Post* post = reinterpret_cast<Post*>(con_cls);
		
			if (post->dataset.size() < offset + size)
			{
				if (offset + size > DATASETSIZE)
					return MHD_NO;

				post->dataset.resize(offset + size + 1);
			}
		
			memcpy(&post->dataset[offset], data, size);
			post->dataset[post->dataset.size() - 1] = '\0';
		}

		return MHD_YES;
	}

	static void finalize(void* cls, struct MHD_Connection* connection,
		void** con_cls, enum MHD_RequestTerminationCode toe)
	{
		Post* post = reinterpret_cast<Post*>(*con_cls);

		if (!post) return;
		
		MHD_destroy_post_processor(post->processor);
		delete post;
		*con_cls = NULL;
	}
};

static int result_404(struct MHD_Connection* connection)
{
	static string message_404 = "<html><body><h2>404 Not found</h2></body></html>";
	
	struct MHD_Response* response = MHD_create_response_from_buffer(
		message_404.size(), &message_404[0], MHD_RESPMEM_MUST_COPY);
	int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return ret;
}

static bool getFile(const char* filename, string& result, string& mime)
{
	if (!www_data) return false;

	const vector<unsigned char>* content = (*www_data)[filename];
	if (!content) return false;
	
	result = string(reinterpret_cast<const char*>(&(*content)[0]), content->size());

	if (!www_data_mime) return true;

	mime = (*www_data_mime)[filename];

	return true;
}

static int callback(void* cls, struct MHD_Connection* connection,
	const char* curl, const char* method, const char* version,
	const char* upload_data, size_t* upload_data_size, void** con_cls)
{
	if (!strcmp(method, "POST"))
	{
		if (!*con_cls)
		{
			Post* post = new Post();
		
			if (!post)
				return MHD_NO;

			post->processor = MHD_create_post_processor(
				connection, POSTBUFFERSIZE, Post::process, (void*)post);

			if (!post->processor)
			{
				delete post;
				return MHD_NO;
			}

			*con_cls = (void*)post;
			
			return MHD_YES;
		}

		Post* post = reinterpret_cast<Post*>(*con_cls);

		if (*upload_data_size != 0)
		{
			MHD_post_process(post->processor, upload_data, *upload_data_size);
			*upload_data_size = 0;
			
			return MHD_YES;
		}
	}

	string result = "";
	string mime = "text/html";

	if (!strcmp(curl, "/"))
	{
		result = indexPage.getHtml();
	}
	else if (!strcmp(curl, "/process"))
	{
		Post* post = reinterpret_cast<Post*>(*con_cls);
		if (!post)
		{
			result =
				"<html>"
				"<h1>Bad Request</h1>"
				"</html>";
		}
		else
		{
			// TODO Add post actions
		}
	}
	else if (!getFile(&curl[1], result, mime))
	{
		return result_404(connection);
	}	

    // Reset when done.
    struct MHD_Response* response = MHD_create_response_from_buffer(
    	result.size(), &result[0], MHD_RESPMEM_MUST_COPY);
	MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, mime.c_str());
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    
    return ret;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("%s <port>\n", argv[0]);
		return 1;
	}
	
	int port = atoi(argv[1]);

	struct MHD_Daemon* daemon;
	
    if (port == 443)
    {
		static SSLKeys sslKeys;

		// Use SSL.
		daemon = MHD_start_daemon(
			MHD_USE_SELECT_INTERNALLY | MHD_USE_SSL, port, NULL, NULL,
			&callback, NULL,
			MHD_OPTION_HTTPS_MEM_KEY, sslKeys.getPrivateKey().c_str(),
			MHD_OPTION_HTTPS_MEM_CERT, sslKeys.getCertificate().c_str(),
			MHD_OPTION_NOTIFY_COMPLETED, Post::finalize, NULL,
			MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int)120,
			MHD_OPTION_END);
	}
	else
	{
		daemon = MHD_start_daemon(
			MHD_USE_SELECT_INTERNALLY, port, NULL, NULL,
			&callback, NULL,
			MHD_OPTION_NOTIFY_COMPLETED, Post::finalize, NULL,
			MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int)120,
			MHD_OPTION_END);
	}	
	
	if (daemon == NULL)
	{
		fprintf(stderr, "Error starting server with port %d, errno = %d\n", port, errno);
		return 1;
	}

	while (1) sleep(1);

	MHD_stop_daemon(daemon);

	return 0;
}

