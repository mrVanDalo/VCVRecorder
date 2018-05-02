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
    uncrustify
    rake
    nix-prefetch-git
    pkgconfig

    # for vcvrecorder
    curl
    glew
    glfw
    gnome2.gtk.dev
    jansson
    libsamplerate
    libzip
    makeWrapper
    pkgconfig
    rtaudio
    rtmidi
    alsaLib
    
  ];

  #shellHook = '''';

}