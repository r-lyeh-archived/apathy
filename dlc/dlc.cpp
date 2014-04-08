
sao::dlc dlc;

dlc.parse("http://localhost:5000/dlcs/");

dlc game_commmons("core", "sounds", ...);
dlc game_devices("iphone4", "iphone5");
dlc game_users(12302, 12392);

// 

auto download = dlc.sync_http_resume();
if( download.bytes() == 0 ) {
	dlc.read_cached_toc();
} else {
	dlc.parse_new_toc();
}

//

auto download = dlc.async_http_resume();

while( download.percent() < 100 ) {
	draw_loading();
}

if( download.bytes() == 0 ) {
	dlc.read_cached_toc();
} else {
	dlc.parse_new_toc();
}


// updates

http game("http://gameweb/game.zip");
http profile("http://gameweb/profiles/iphone4.zip");
http usercontent("http://gameweb/users/123.zip");

if( game.resume() || profile.resume() || usercontent.resume() ) {
	toc = toc( {game.toc(), profile.toc(), usercontent.toc()} );
	toc.save();
}

update_packages( {"game.common","game.profile.iphone4","game.user.12312"} );


// optional (packages)
pacman.init( options );
pacman.query_remote_packages();
pacman.list_local_packages();
pacman.list_remote_packages();
pacman.install_packages({});
pacman.update_packages({}); // will install if it does not exist
pacman.uninstall_packages({});

// assets

{
  "commit": ,
}

// toc, assets

{ 
  "name",
  "extension":,
  "id": num,
  "type": ,
  "offset":
  "size":,
  "requires": [id..]
  "volume": ,
  "placeholder": id
  "platform": 
}

// dlc 

{ 
   "mirrors" : [ uri, uri ... ],
   "commits": []
}
