import numpy as np

class Link:
    def __init__(self, lat1: float, lon1: float, lat2: float, lon2: float, rssi: float):
        self.node1 = [lat1, lon1]
        self.node2 = [lat2, lon2]
        self.rssi = float(rssi)

    def __eq__(self, other):
        return (self.node1[0] == other.node1[0] and self.node1[1] == other.node1[1] and self.node2[0] == other.node2[0]
                and self.node2[1] == other.node2[1]) or (
                       self.node2[0] == other.node1[0] and self.node2[1] == other.node1[1] and self.node1[0] ==
                       other.node2[0]
                       and self.node1[1] == other.node2[1])


class LinkPair:
    def __init__(self, link1: Link, link2: Link):
        self.link1 = link1
        self.link2 = link2
        self.angle = self.angle_between()
        self.rssi = (link1.rssi + link2.rssi) / 2

    def __eq__(self, other):
        return (self.link1 == other.link1 and self.link2 == other.link2) or (
                self.link1 == other.link2 and self.link2 == other.link1)

    def angle_between(self) -> float:
        if self.link1.node1 == self.link2.node1:
            origin = self.link1.node1
            loc1 = self.link1.node2
            loc2 = self.link2.node2

        elif self.link1.node1 == self.link2.node2:
            origin = self.link1.node1
            loc1 = self.link1.node2
            loc2 = self.link2.node1

        elif self.link1.node2 == self.link2.node1:
            origin = self.link1.node2
            loc1 = self.link1.node1
            loc2 = self.link2.node2

        else:
            origin = self.link1.node2
            loc1 = self.link1.node1
            loc2 = self.link2.node1

        a = np.array([float(i) for i in origin])
        b = np.array([float(i) for i in loc1])
        c = np.array([float(i) for i in loc2])

        ba = a - b
        bc = c - b

        cosine_angle = np.dot(ba, bc) / (np.linalg.norm(ba) * np.linalg.norm(bc))
        angle = np.arccos(cosine_angle)
        return np.degrees(angle)