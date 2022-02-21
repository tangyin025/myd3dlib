
class AStar:
    def __init__(self):
        self.open=set()
        self.close=set()
        self.gscore={}
        self.fscore={}
        self.path={}
