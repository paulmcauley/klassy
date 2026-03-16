{
  description =
    "Klassy - Theming utility for the KDE Plasma desktop environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        package = pkgs.callPackage ./package.nix { };
      in {
        packages = {
          default = package;
          klassy = package;
        };

        devShells.default = pkgs.mkShell { inputsFrom = [ package ]; };
      });
}
