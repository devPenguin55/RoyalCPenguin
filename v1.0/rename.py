import os 

for i in os.listdir("GrandmasterPGNs"):
    i = os.path.join("GrandmasterPGNs", i)
    os.rename(i, i.replace('.pgn', 'e.txt'))