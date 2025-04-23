#!/bin/bash

data_dir="data"
log_dir="logs"
solver="./Release/gurobiSolverCWLP"

if [ ! -f "$solver" ]; then
  echo "Erreur : l'exécutable $solver est introuvable"
  exit 1
fi

mkdir -p "$log_dir"

# Boucle sur les combinaisons
for X in C R; do
  for Y in {1..8}; do
    for Z in {1..10}; do
      for W in {1..9}; do
        for v in 0 2; do 
          filename="bologne-${X}1-BPPC_${Y}_0_${Z}.txt_0.${W}"
          filepath="${data_dir}/${filename}"
          logfile="${log_dir}/${filename}_model${v}.log"

          # Ne pas relancer si log existe déjà
          if [ -f "$logfile" ]; then
            echo "Déjà traité : $filename avec modèle $v"
            continue
          fi

          if [ -f "$filepath" ]; then
            echo "→ Lancement : $filename avec modèle $v"
            "$solver" "$filename" "$v" > "$logfile" 2>&1
          else
            echo "Fichier introuvable : $filepath"
          fi
        done
      done
    done
  done
done
