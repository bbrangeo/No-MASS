#!/bin/bash
# Script pour compiler tous les fichiers .md en un seul PDF

# Nom de sortie
output="resultat.pdf"

# Lister tous les fichiers .md
files=$(ls *.md | sort)

# Compiler avec Pandoc
pandoc $files -o $output --toc --pdf-engine=/Library/TeX/texbin/pdflatex

echo "Compilation terminée. Fichier généré : $output"

