# Nix-shell script
# ----------------

with import <nixpkgs> {};

stdenv.mkDerivation rec {

  name = "vcvrecorder";

  env = buildEnv {
    name = name;
    paths = buildInputs;
  };

  buildInputs = [
    jetbrains.clion
    uncrustify
    rake
    nix-prefetch-git
  ];

  #shellHook = '''';

}