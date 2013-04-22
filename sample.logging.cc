#include <cstdio>
#include <iostream>
#include <string>
#include "sao.hpp"
using namespace sao;

void my_custom_logger( bool open, bool feed, bool close, const std::string &line )
{
    static FILE *fp;
    static std::string cache;
    static unsigned linenumber = 0;

    if( open )
    {
        fp = std::fopen("log.html", "wb");

        const char *jscript =
        "<script>format = function() {"
        "document.body.innerHTML = '<pre>' + document.body.innerHTML.replace(/>/g,'&gt;').replace(/</g,'&lt;').replace(/\"/g,'&quot;') + '</pre>';"
        "}</script>";

        std::fprintf( fp, "%s", "<!-- generated file -->\n" );
        std::fprintf( fp, "<html><head>%s</head><body onload='replace_all()'>\n<pre>", jscript );
    }
    else
    if( close )
    {
        std::fprintf( fp, "%s", ( cache + "\n</pre></body></html>\n<!-- closing log file... -->\n" ).c_str() );
        std::fclose( fp );

        cache = std::string();
    }
    else
    if( feed )
    {
        std::fprintf( fp, "[*] line %d: %s\n", ++linenumber, cache.c_str() );

        cache = std::string();
    }
    else
    {
        cache += line;
    }
}

int main( int argc, char **argv )
{
    std::cout << "next lines are going to be redirected to a 'log.html' file ..." << std::endl;

    stream::attach( std::cout, my_custom_logger );

    std::cout << "this is a line redirected from standard std::cout stream to a custom HTML file logger" << std::endl;
    std::cout << "this is a line redirected from standard std::cout stream to a custom HTML file logger" << std::endl;

    stream::detach( std::cout );

    std::cout << "... back from redirecting. Please check 'log.html' file." << std::endl;

    return 0;
}
