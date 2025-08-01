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



e = '''
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1|1|20
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1|2|400
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1|3|8902
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1|4|197281
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1|5|4865609
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1|6|119060324
8/8/1k6/8/2pP4/8/5BK1/8 b - d3 0 1|6|824064
8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1|6|1440467
8/5k2/8/2Pp4/2B5/1K6/8/8 w - d6 0 1|6|1440467
5k2/8/8/8/8/8/8/4K2R w K - 0 1|6|661072
4k2r/8/8/8/8/8/8/5K2 b k - 0 1|6|661072
3k4/8/8/8/8/8/8/R3K3 w Q - 0 1|6|803711
r3k3/8/8/8/8/8/8/3K4 b q - 0 1|6|803711
r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1|4|1274206
r3k2r/7b/8/8/8/8/1B4BQ/R3K2R b KQkq - 0 1|4|1274206
r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1|4|1720476
r3k2r/8/5Q2/8/8/3q4/8/R3K2R w KQkq - 0 1|4|1720476
2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1|6|3821001
3K4/8/8/8/8/8/4p3/2k2R2 b - - 0 1|6|3821001
8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1|5|1004658
5K2/8/1Q6/2N5/8/1p2k3/8/8 w - - 0 1|5|1004658
4k3/1P6/8/8/8/8/K7/8 w - - 0 1|6|217342
8/k7/8/8/8/8/1p6/4K3 b - - 0 1|6|217342
8/P1k5/K7/8/8/8/8/8 w - - 0 1|6|92683
8/8/8/8/8/k7/p1K5/8 b - - 0 1|6|92683
K1k5/8/P7/8/8/8/8/8 w - - 0 1|6|2217
8/8/8/8/8/p7/8/k1K5 b - - 0 1|6|2217
8/k1P5/8/1K6/8/8/8/8 w - - 0 1|7|567584
8/8/8/8/1k6/8/K1p5/8 b - - 0 1|7|567584
8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1|4|23527
8/5k2/8/5N2/5Q2/2K5/8/8 w - - 0 1|4|23527
r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1|5|193690690
8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1|6|11030083
r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1|5|15833292
rnbqkb1r/pp1p1ppp/2p5/4P3/2B5/8/PPP1NnPP/RNBQK2R w KQkq - 0 1|3|53392
r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 1|5|164075551
8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1|7|178633661
r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1|6|706045033
rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8|5|89941194
1k6/1b6/8/8/7R/8/8/4K2R b K - 0 1|5|1063513
3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1|6|1134888
8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1|6|1015133
'''

e = e.strip().split('\n')
for i in e:
    i = i.split("|")
    print(f'''
    initBoard(&board, "{i[0]}", &tt);
    perft = moveGenerationTest(&board, {i[1]});
    if (perft == {i[2]}) {{
        printf("\\nPassed Test Case of Fen %s on depth %d", "{i[0]}", {i[1]});
    }} else {{
        printf("\\n\\n      FAILED Test Case of Fen %s on depth %d", "{i[0]}", {i[1]});
    }}
    ''')