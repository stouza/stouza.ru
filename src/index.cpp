#include "index.h"

#include <map>
#include <memory>
#include <vector>

using namespace std;

// Container for embedded content that shall be
// loaded and persist in memory during the server lifetime.
extern unique_ptr<map<string, vector<unsigned char>*> > www_data;

const string& Index::getHtml()
{
	if (initialized) return html;

	if (!www_data) return html;

	const vector<unsigned char>* content = (*www_data)["index.html.in"];
	if (!content) return html;

	html = string(reinterpret_cast<const char*>(&(*content)[0]),
		content->size());
	
	// TODO Use regex
	string gitSHA1 = GIT_SHA1;
	string placeholder = "__VERSION_NUMBER__";
	auto found = html.find(placeholder);
	if (found != string::npos)
		html.replace(found, placeholder.length(), gitSHA1);
	
	initialized = true;
	return html;
}

Index::Index() : initialized(false) { }

