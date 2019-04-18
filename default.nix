with import <nixpkgs> { }; {
	lol = stdenvNoCC.mkDerivation {
		name = "foobla";
		buildInputs = [
			SDL2 SDL2_image pkg-config glm qt5.full gcc8 gnumake curl
		];
	};
}
