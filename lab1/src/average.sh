#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Ошибка: Не переданы аргументы"
    exit 1
fi

count=$#
sum=0

for num in "$@"; do
    sum=$((sum + num))
done

average=$((sum / count))

echo "Количество чисел: $count"
echo "Среднее арифметическое: $average"
