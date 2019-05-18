#!/usr/bin/env bash
echo "Generating Synthetic project files"
echo "----------------------"
"$UE4_ROOT"/GenerateProjectFiles.sh -project="$UE4_SYNTHETIC"/Synthetic.uproject -game -engine -Makefile -vscode
