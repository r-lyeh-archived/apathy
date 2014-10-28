namespace game {
    struct texture {
        int id;

        texture()
        {}

        texture( const void *ptr, size_t len ) : id(0) {
            if( ptr ) {
                // do whatever
                // glBindTexture
            } else {
                id = 0; // placeholder
            }
        }
    };
};

desserts("file globbing") {
    auto list = apathy::lsfr("*.cxx;");
    dessert( list.size() > 0 );
}

desserts("absolute file globbing") {
    uris files;
    uris dirs;

#ifdef _WIN32
    std::string tmpdir = getenv("TEMP") ? getenv("TEMP") : "";
#else
    std::string tmpdir = getenv("TMPDIR") ? getenv("TMPDIR") : "";
#endif

    if( !tmpdir.empty() ) {
        files.clear();
        dirs.clear();
        dessert( apathy::mkdir(tmpdir + "/apathy") );
        dessert( apathy::exists(tmpdir + "/apathy") );
        dessert( apathy::rmdir( tmpdir + "/apathy" ) );

        dessert( apathy::mkdir(tmpdir + "/apathy") );
        dessert( apathy::globr(tmpdir, files, dirs) );
        assert( files.find(tmpdir + "/apathy") == files.end() );
      //assert( dirs.find(tmpdir + "/apathy") != dirs.end() );
    }

#ifdef _WIN32
    files.clear();
    dirs.clear();
    dessert( apathy::glob("C:", files, dirs) );
     assert( files.find("apathy.cpp") != files.end() );
     assert( dirs.find("redist") != files.end() );

    files.clear();
    dirs.clear();
    dessert( apathy::glob("C:/", files, dirs) );
     assert( files.find("apathy.cpp") == files.end() );
     assert( dirs.find("redist") == files.end() );
#endif
}


desserts("folder monitoring") {
#if 0
    std::cout << "save " << __FILE__ << " to exit..." << std::endl;
    apathy::watch( "./", [&]( apathy::watcher &wt ) {
        std::cout << "\tin:" << std::endl;
        apathy::print( wt.in );

        std::cout << "\tout:" << std::endl;
        apathy::print( wt.out );

        std::cout << "\tmodif:" << std::endl;
        apathy::print( wt.modif );

        wt.sleep( 1.0 );
        wt.reload();
    } );

    for(;;) {
        apathy::sleep(1.0);
    }
#endif
}

desserts("filesystems") {
    apathy::vfilesystem vfs;

    // register std_io_filesystem to handle "file://" and empty protocol urls
    vfs.add_filesystem<apathy::std_io_filesystem>( {"file://", ""} );

    // test 
    game::texture tex;
    tex = vfs.make<game::texture>( "test.tga" );
    vfs.make( tex, "test.tga" );

    vfs.mount("tests/*", "my_tests/");
    vfs.mount("music.zip/*", "music/");
    vfs.mount("textures/*", "icons/");
    vfs.mount("*.c*", "code/");
    vfs.mount("**.cpp", "code-r/");

    // flat paths
    vfs.mount("dlc-watchman/*.txt", "/" );

    dessert( vfs.exists("code/tests.cxx") );
    dessert( vfs.exists("file://code/tests.cxx") );

    std::cout << vfs.toc() << std::endl;

    dessert( vfs.read("file://code/tests.cxx") == apathy::file(__FILE__).read() );
}
