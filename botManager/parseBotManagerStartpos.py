import chess
import chess.pgn
import random as r
from tqdm import tqdm
from stockfish import Stockfish

stockfish = Stockfish(path=r"C:\AaravC-Projects\Chess Engine\botManager\stockfish.exe", depth=8)

def boardFairlyEqual(board:chess.Board):
    stockfish.set_fen_position(board.fen())
    evaluation = stockfish.get_evaluation()['value']
    # a fairly equal position is between -0.3 and 0.3, take abs to make easier
    evaluation = abs(evaluation) / 100
    return evaluation <= 0.3 

games = open(r"C:\AaravC-Projects\Chess Engine\botManager\lichess_db_standard_rated_2015-09.pgn")
finalFens = []
amt = 300_000
with tqdm(total=amt) as pbar:
    for _ in range(amt):
        game = chess.pgn.read_game(games)
        board = game.board()

        movesPlayed = 0
        stopMovesPlayed = r.randint(10, 36)
        for move in game.mainline_moves():
            board.push(move)
            movesPlayed += 1
            if movesPlayed == stopMovesPlayed:
                break
        pbar.update()
        if movesPlayed >= stopMovesPlayed:
            # play some moves to the board from the position
            # now ask stockfish for the eval to see if the position is equal 
            if boardFairlyEqual(board):
                finalFens.append(board.fen())
                pbar.set_postfix({"valid positions":len(finalFens)})

with open("botManagerStartpos.txt", "w+") as f:
    f.seek(0)
    for i, fen in enumerate(finalFens):
        f.write(f"{fen}{"\n" if i != len(finalFens)-1 else ""}")