stockfish = '''
b7h1: 3554
b7g2: 6348
b7f3: 7150
b7e4: 7433
b7d5: 9394
b7a6: 5280
b7c6: 8465
b7a8: 5720
b7c8: 5816
b8a7: 6781
b8c7: 8450
b8a8: 5161
b8c8: 6213
'''

mine = '''
b8a8 -> 5161
b8c8 -> 6213
b8a7 -> 6781
b8c7 -> 8450
b7a8 -> 5720
b7c8 -> 5816
b7a6 -> 5280
b7c6 -> 8465
b7d5 -> 9394
b7e4 -> 7432
b7f3 -> 7149
b7g2 -> 6348
b7h1 -> 3554
'''

stockfish = stockfish.strip().split("\n")
print(stockfish)
finalStockfish = {}
for i in stockfish:
    i = i.split(": ")
    finalStockfish[i[0]] = int(i[1])


mine = mine.strip().split("\n")

finalMine = {}
for i in mine:
    i = i.split(" -> ")

    finalMine[i[0]] = int(i[1])

for key in finalStockfish.keys():
    if finalStockfish[key] != finalMine[key]:
        print(key, "stockfish got", finalStockfish[key], "but I got", finalMine[key])