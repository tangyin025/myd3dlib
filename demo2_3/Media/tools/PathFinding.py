import sys

class AStar:
    def __init__(self,start,goal,limit):
        self.cleanup()
        self.start=start
        self.goal=goal
        self.limit=limit

    def cleanup(self):
        self.open=set()
        self.close=set()
        self.gscore={}
        self.fscore={}
        self.came_from={}

    def solve(self):
        self.cleanup()
        self.open.add(self.start)
        self.gscore[self.start]=0
        self.fscore[self.start]=self.heuristic_cost_estimate(self.start,self.goal)
        while len(self.open)>0:
            curr=self.the_node_in_open_having_the_lowest_fScore_value()
            if curr==self.goal:
                return True
            elif len(self.close)>self.limit:
                return False
            self.open.remove(curr)
            self.close.add(curr)
            neis=self.get_neighbors(curr)
            nei_it=iter(neis)
            for nei in nei_it:
                if nei in self.close:
                    continue
                if nei not in self.open:
                    self.open.add(nei)
                    assert(nei not in self.gscore)
                    self.gscore[nei]=sys.float_info.max
                tentative_gscore=self.gscore[curr]+self.dist_between(curr,nei)
                if tentative_gscore>self.gscore[nei]:
                    continue
                self.gscore[nei]=tentative_gscore
                self.fscore[nei]=tentative_gscore+self.heuristic_cost_estimate(nei,self.goal)
                self.came_from[nei]=curr
        return False

    def the_node_in_open_having_the_lowest_fScore_value(self):
        lowest_score=sys.float_info.max
        lowest_pos=None
        it=iter(self.open)
        for pos in it:
            if pos in self.fscore:
                score=self.fscore[pos]
                if score<lowest_score:
                    lowest_score=score
                    lowest_pos=pos
        assert(lowest_pos is not None)
        return lowest_pos

    def heuristic_cost_estimate(self,start,goal):
        return 0

    def get_neighbors(self,pos):
        return []

    def dist_between(self,start,goal):
        return 0
