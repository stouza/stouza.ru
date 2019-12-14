#ifndef INDEX_H
#define INDEX_H

#include <string>

class Index
{
	bool initialized;
	std::string html;

public :

	const std::string& getHtml();

	Index();
};

#endif // INDEX_H

