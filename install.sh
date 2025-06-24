#!/bin/bash

set -e

INSTALL_DIR="$(pwd)/src"

echo "Instalando en $INSTALL_DIR"

# Dar permisos de ejecución a los archivos necesarios
chmod +x "$INSTALL_DIR/SaukOS"
chmod +x "$INSTALL_DIR/Tetris/tetris.sh"

# Detectar archivo de configuración de shell
if [ -n "$BASH_VERSION" ]; then
    PROFILE_FILE="$HOME/.bashrc"
elif [ -n "$ZSH_VERSION" ]; then
    PROFILE_FILE="$HOME/.zshrc"
else
    PROFILE_FILE="$HOME/.profile"
fi

# Agregar src al PATH si no está ya
if ! grep -Fxq "export PATH=\"$INSTALL_DIR:\$PATH\"" "$PROFILE_FILE"; then
    echo "export PATH=\"$INSTALL_DIR:\$PATH\"" >> "$PROFILE_FILE"
    echo "Agregado al PATH: $INSTALL_DIR"
else
    echo "El PATH ya incluye $INSTALL_DIR"
fi

echo "Instalación completada."
echo "Iniciando SaukOS . . . "

"$INSTALL_DIR/SaukOS"

