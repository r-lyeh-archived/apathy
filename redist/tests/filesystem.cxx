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
    vfs.mount("*.cpp", "code/");
    vfs.mount("**.cpp", "code-r/");

    // flat paths
    vfs.mount("dlc-watchman/*.txt", "/" );

    dessert( vfs.exists("code/extra.cpp") );
    dessert( vfs.exists("file://code/extra.cpp") );

    std::cout << vfs.toc() << std::endl;

    dessert( vfs.read("file://code/extra.cpp") == apathy::file(__FILE__).read() );
}
