#!/bin/bash

# Проверяем, есть ли аргументы
if [ $# -eq 0 ]; then
    echo "No arguments provided"
    exit 1
fi

count=$#
sum=0

# Суммируем все аргументы
for num in "$@"; do
    sum=$(echo "$sum + $num" | bc)
done

# Вычисляем среднее арифметическое
average=$(echo "scale=2; $sum / $count" | bc)

echo "Count of numbers: $count"
echo "Average: $average"
