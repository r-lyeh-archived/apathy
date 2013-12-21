/* Simple http request class
 * Based on code by Vijay Mathew Pandyalakal
 *
 * - rlyeh
 */

#pragma once

#include <map>
#include <string>

namespace sao
{
	class http
	{
		public:

			std::map<std::string,std::string> vars;
			std::string host, path, response, error;

			 http();
			~http();

			bool connect();
			bool send();

		private:

			struct impl;
			impl *pImpl;

			// disable copy & assignment
			http( const http &other );
			http &operator =( const http &other );
	};

	std::string download( const std::string &url );
}
