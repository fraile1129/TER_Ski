#!/bin/bash

data_dir="data"
log_dir="logs"
solver="./Release/gurobiSolverCWLP"
fres="test.txt"

if [ ! -f "$solver" ]; then
  echo "Erreur : l'exécutable $solver est introuvable"
  exit 1
fi

if [ ! -f "$fres" ]; then
  echo "Nom;Version;BP;BD;Gap;Temps" > $fres
fi


# Boucle sur les combinaisons
for Y in 5 1 6 2 7 3 8 4; do # taille, nb de sommets # 5-60 1/6-120 2/7-250 3/8-50 4-1000
  for W in {1..9}; do # nb de arcs (densité)
    for Z in {1..3}; do # instance diff
      for v in -1 0 2 11; do 
        for X in C R; do
          filename="bologne-${X}1-BPPC_${Y}_0_${Z}.txt_0.${W}"
          filepath="${data_dir}/${filename}"
          #logfile="${log_dir}/${filename}_model${v}.log"

          # Ne pas relancer si log existe déjà
          # if [ -f "$logfile" ]; then
          #   echo "Déjà traité : $filename avec modèle $v"
          #   continue
          # fi

          if [ -f "$filepath" ]; then
            echo "→ Lancement : $filename avec modèle $v"
            "$solver" "$filename" "$v" #> "$logfile" 2>&1
          else
            echo "Fichier introuvable : $filepath"
          fi
        done
      done
    done
  done
done
