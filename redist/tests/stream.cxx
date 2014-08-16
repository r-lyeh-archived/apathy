
desserts() {
    apathy::stream s;
    assert( !s.open() );
    assert( !s.close() );
    assert( s.size() == 0 );
    assert( s.tell() == 0 );
    assert( s.left() == 0 );
    assert( s.eof() == true );
    assert( s.rewind() == false );
    assert( s.offset(0) == false );
    assert( s.seek(0) == false );
    assert( s.offset(~0) == false );
    assert( s.seek(~0) == false );
    int i;
    assert( s.read(i) == false );
    assert( s.write(i) == false );

    {
        const char *hello_world = "hello world";
        s = apathy::stream( hello_world, strlen(hello_world) );
        assert( s.open() );
        assert(!s.write32('1234') );
        assert( std::string(hello_world) == "hello world" );
    }
    {
        char hello_world[] = "hello world";
        s = apathy::stream( hello_world, strlen(hello_world) );
        assert( s.open() );
        int32_t val = '1234';
        assert( s.write32(val) ); // host to le
        assert( std::string(hello_world) == "4321o world" || std::string(hello_world) == "1234o world" );
        assert( s.rewind() );
        assert( s.read(val) );
        assert( val == '1234' );
    }
    {
        std::string h( "son of a " );
        for (int i = 0; i < 2; ++i) h += h;
        s = apathy::stream( &h[0], h.size() );
        std::cout << s << std::endl;
    }
}
