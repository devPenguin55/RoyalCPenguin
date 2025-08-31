import os
import chess

pgns = []
for file in os.listdir("GrandmasterPGNs"):
    file = os.path.join("GrandmasterPGNs", file)
    with open(file, "r") as f:
        data = [i.strip() for i in f.readlines()]
        packet = []
        for line in data:
            if ("[" in line) or ("]" in line) or (not line.strip()):
                if packet: 
                    packet = ' '.join(packet)
                    packet = [i.strip() for i in packet.split(" ") if '.' not in i]
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

with open("book.txt", "w+") as bookFile:
    bookFile.seek(0)
    fenToPossibleMoves = {}
    for index, pgn in enumerate(pgns):
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
            e = e
    
    for index, fen in enumerate(fenToPossibleMoves):
        moves = fenToPossibleMoves[fen]
        moves = {key:moves[key] for key in sorted(moves, key=lambda x: moves[x], reverse=True)}
        bookFile.write(fen+": ")
        for moveIdx, move in enumerate(moves):
            bookFile.write(f'{move} ({moves[move]})')
            if moveIdx != len(moves) - 1:
                bookFile.write(", ")

        if index != len(fenToPossibleMoves) - 1:
            bookFile.write("\n")
        