f_solveur=../results/results_solveur.txt
f_sous_gradients=../results/results_sous_gradients

K=(5 20 50 100)
alpha=(1.9 1.5 1 0.5 0.2)


a=("1" "2" "3" "5" "6" "7" "8")
b=("0.2" "0.5")

echo "Nom nbItems Valeur Temps" > $f_solveur

for n in "${a[@]}"; do
    for m in "${b[@]}"; do
        echo "Solveur instance C10_BPPC_${n}_0_1_${m}"
        ./exe C10_BPPC_${n}_0_1_${m} $f_solveur
    done
done

for k in "${K[@]}"; do
    for al in "${alpha[@]}"; do
        echo "Nom nbItems Valeur Temps" > "${f_sous_gradients}_K_${k}_Alpha_${al}.txt"
        for n in "${a[@]}"; do
            for m in "${b[@]}"; do
                echo "Algo de sous-gradients instance C10_BPPC_${n}_0_1_${m}"
                echo "K = ${k}, alpha = ${al}"
                ./exe C10_BPPC_${n}_0_1_${m} "${f_sous_gradients}_K_${k}_Alpha_${al}.txt" $al $k
            done
        done
    done
done