with import <nixpkgs> { }; {
	lol = stdenvNoCC.mkDerivation {
		name = "foobla";
		buildInputs = [
			SDL2 SDL2_image SDL2_ttf pkg-config glm qt5.full gcc8 gdb gnumake 
curl
		];
	};
}
