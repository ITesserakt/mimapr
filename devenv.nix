{ pkgs, lib, config, inputs, ... }:

{
  packages = with pkgs; [
  	cmake
  	gnumake
  	clang
  ];
}
