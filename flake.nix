{
  description = "A language server for GLSL";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        bshoshany-thread-pool = pkgs.fetchFromGitHub {
          owner = "bshoshany";
          repo = "thread-pool";
          rev = "v4.1.0";
          sha256 = "sha256-zhRFEmPYNFLqQCfvdAaG5VBNle9Qm8FepIIIrT9sh88=";
        };

      in
      {
        packages.default = pkgs.llvmPackages_19.stdenv.mkDerivation {
          pname = "glsld";
          version = "0.1.0";
          src = ./.;

          nativeBuildInputs = with pkgs; [
            cmake
            ninja
            llvmPackages_19.clang
          ];

          buildInputs = with pkgs; [
            fmt
            spdlog
            argparse
            nlohmann_json
            boost
            magic-enum
            catch2_3
          ];

          cmakeFlags = [
            "-DCMAKE_BUILD_TYPE=Release"
            "-DCMAKE_CXX_COMPILER=clang++"
            "-DGLSLD_BUILD_WRAPPER=ON"
            "-DGLSLD_BUILD_LANGUAGE_SERVER=ON"
            "-DGLSLD_BUILD_UNIT_TEST=ON"
            "-DBSHOSHANY_THREAD_POOL_INCLUDE_DIRS=${bshoshany-thread-pool}/include"
          ];

          NIX_CFLAGS_COMPILE = "-I${bshoshany-thread-pool}/include";

          preConfigure = ''
            mkdir -p build/vcpkg
            cat > build/vcpkg/toolchain.cmake << 'EOF'
            set(CMAKE_CXX_STANDARD 23)
            EOF

            mkdir -p build/cmake/boost_pfr
            cat > build/cmake/boost_pfr/boost_pfrConfig.cmake << 'EOF'
            if(NOT TARGET Boost::pfr)
              add_library(boost_pfr_pfr INTERFACE IMPORTED)
              set_target_properties(boost_pfr_pfr PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${pkgs.boost.dev}/include"
              )
              add_library(Boost::pfr ALIAS boost_pfr_pfr)
              set(boost_pfr_FOUND TRUE)
            endif()
            EOF

            cmakeFlags="$cmakeFlags -DCMAKE_TOOLCHAIN_FILE=$PWD/build/vcpkg/toolchain.cmake"
            cmakeFlags="$cmakeFlags -DCMAKE_PREFIX_PATH=$PWD/build/cmake"
          '';

          preBuild = ''
            ninja glsld-lexgen
            ./glsld-lexgen Tokenize.generated.cpp
          '';

          doCheck = true;

          installPhase = ''
            runHook preInstall
            mkdir -p $out/bin
            install -m755 glsld glsld-wrapper $out/bin/
            runHook postInstall
          '';

          meta = with pkgs.lib; {
            description = "A language server for GLSL";
            homepage = "https://github.com/daiyousei-qz/glsld";
            license = licenses.mit;
            platforms = platforms.unix ++ platforms.windows;
          };
        };

        devShells.default = pkgs.mkShell {
          inputsFrom = [ self.packages.${system}.default ];

          shellHook = ''
            echo "GLSLD development environment"
            echo "Clang: $(clang++ --version | head -n1)"
            echo "CMake: $(cmake --version | head -n1)"
            export CC=clang
            export CXX=clang++
          '';
        };

        apps.default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/glsld";
        };
      }
    );
}
