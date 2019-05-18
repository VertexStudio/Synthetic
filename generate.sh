#!/usr/bin/env bash
echo "Generating UnrealGAMS project files"
echo "----------------------"
"$UE4_ROOT"/GenerateProjectFiles.sh -project="$UE4_SYNTHETIC"/Synthetic.uproject -game -engine -Makefile -vscode
