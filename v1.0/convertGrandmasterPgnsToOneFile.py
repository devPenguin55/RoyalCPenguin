import os
import chess
from tqdm import tqdm 

pgns = []
print("Reading contents of files")
for file in tqdm(os.listdir("GrandmasterPGNs")):
    file = os.path.join("GrandmasterPGNs", file)
    with open(file, "r") as f:
        data = [i.strip() for i in f.readlines()]
        packet = []
        for line in data:
            if (("[" in line) or ("]" in line) or (not line.strip())) and ("{[%clk" not in line):
                if packet: 
                    packet = ' '.join(packet)
                    packet = [i.strip() for i in packet.split(" ") if '.' not in i and '{[%clk' not in i and ":" not in i and i]
                    packet = packet[:min(16, len(packet))]
                    pgns.append(packet)
                packet = []
                
            else:
                packet.append(line)
        if packet: 
            packet = ' '.join(packet)
            packet = [i.strip() for i in packet.split(" ") if '.' not in i]
            packet = packet[:min(16, len(packet))]
            pgns.append(packet)


print("Processing PGNs and Counting Moves per FEN")
with open("book.txt", "w+") as bookFile:
    bookFile.seek(0)
    fenToPossibleMoves = {}
    for index, pgn in tqdm(enumerate(pgns), total=len(pgns)):
        try:
            board = chess.Board()
            for move in pgn:
                originalFen = board.fen()
                uci = board.push_san(move) # * e2e4 instead of e2  (can encode now)
                encodedMove = (uci.from_square) + ((uci.to_square)<<6) + ((uci.promotion if uci.promotion else 0)<<12)
                if fenToPossibleMoves.get(originalFen):
                    if fenToPossibleMoves[originalFen].get(encodedMove):
                        fenToPossibleMoves[originalFen][encodedMove] += 1
                    else:
                        fenToPossibleMoves[originalFen][encodedMove] = 1
                else:
                    fenToPossibleMoves[originalFen] = {encodedMove:1}
        except Exception as e:
            # print(e)
            e = e
    
    print("Writing fen book to file...")
    for index, fen in tqdm(enumerate(fenToPossibleMoves), total=len(list(fenToPossibleMoves.keys()))):
        moves = fenToPossibleMoves[fen]
        moves = {key:moves[key] for key in sorted(moves, key=lambda x: moves[x], reverse=True)}
        bookFile.write(fen+": ")
        for moveIdx, move in enumerate(moves):
            bookFile.write(f'{move} ({moves[move]})')
            if moveIdx != len(moves) - 1:
                bookFile.write(", ")

        if index != len(fenToPossibleMoves) - 1:
            bookFile.write("\n")
        