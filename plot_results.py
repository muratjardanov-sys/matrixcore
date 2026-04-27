# -*- coding: cp1251 -*-
import pandas as pd
import matplotlib.pyplot as plt
import glob

files = glob.glob("res_*.csv")

for file in files:
    df = pd.read_csv(file)
    plt.figure(figsize=(10, 6))
    
    plt.plot(df['PathLength'], df['Sequential'], label='C++ Последоват.', marker='o')
    plt.plot(df['PathLength'], df['Parallel'], label='C++ Параллел. (4 потока)', marker='s')
    plt.plot(df['PathLength'], df['SQL'], label='SQLite JOIN', marker='^')
    
    plt.yscale('log') 
    plt.title(f"Сравнение производительности: {file}")
    plt.xlabel("Длина пути (k)")
    plt.ylabel("Время (секунды)")
    plt.legend()
    plt.grid(True, which="both", ls="--")

plt.show() 