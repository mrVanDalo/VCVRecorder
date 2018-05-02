# function header
# ---------------
{ pkgs ? import <nixpkgs> {} }:

# function call
# -------------
(pkgs.buildFHSUserEnv {

  name = "vcvrecorder";

  # targetSystem packages
  # ---------------------
  # these are packages which are compiled for the target
  # system architecture
  targetPkgs = pkgs: with pkgs; [

    # generally needed
    uncrustify
    rake
    nix-prefetch-git
    git
    gnumake
    gcc

    # for vcvrack
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

  ];

  # multilib packages
  # -----------------
  # these are packages compiled for multiple system
  # architectures (32bit/64bit)
  multiPkgs = pkgs: with pkgs; [
  ];

  # environment variables
  # ---------------------
  profile = ''
    export TERM="xterm"
  '';

}).env