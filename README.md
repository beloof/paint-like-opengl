# Paint-like OpenGL

Éditeur d'images simple en OpenGL (GLUT/GLU).

## Build (Windows, VS Code + CMake Tools)

1. Ouvrir le dossier dans VS Code.
2. `Ctrl+Shift+P` → `CMake: Configure`, choisir le kit MSVC.
   Si vcpkg n'est pas auto-détecté, ajouter dans `.vscode/settings.json` :
   ```json
   {
     "cmake.configureSettings": {
       "CMAKE_TOOLCHAIN_FILE": "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
     }
   }
   ```
3. `Ctrl+Shift+P` → `CMake: Build`.
4. Lancer l'exécutable généré dans `build/` — une fenêtre grise avec un triangle doit s'afficher.
