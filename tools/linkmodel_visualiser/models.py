import numpy as np
import math

np.seterr(all='raise')


class Link:
    def __init__(self, lat1: str, lon1: str, lat2: str, lon2: str, rssi: str):
        self.node1 = [float(lat1), float(lon1)]
        self.node2 = [float(lat2), float(lon2)]
        self.rssi = float(rssi) - (-self._l_d(self._distance_between()))

    def __eq__(self, other):
        return (self.node1[0] == other.node1[0] and self.node1[1] == other.node1[1] and self.node2[0] == other.node2[0]
                and self.node2[1] == other.node2[1]) \
               or (self.node2[0] == other.node1[0]
                   and self.node2[1] == other.node1[1]
                   and self.node1[0] == other.node2[0]
                   and self.node1[1] == other.node2[1])

    def _l_d(self, distance: float) -> float:
        return 0.0 if distance == 0 else 55 * math.log10(1000 * distance) - 18

    def _distance_between(self) -> float:
        earth_radius = 6373.0

        lat1 = math.radians(self.node1[0])
        lon1 = math.radians(self.node1[1])
        lat2 = math.radians(self.node2[0])
        lon2 = math.radians(self.node2[1])

        dlon = lon2 - lon1
        dlat = lat2 - lat1

        a = math.sin(dlat / 2) ** 2 + math.cos(lat1) * math.cos(lat2) * math.sin(dlon / 2) ** 2
        c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))

        return earth_radius * c


class LinkPair:
    def __init__(self, link1: Link, link2: Link):
        self.link1 = link1
        self.link2 = link2
        self.angle = self.angle_between()
        self.rssi = link1.rssi - link2.rssi if link1.rssi > link2.rssi else link2.rssi - link1.rssi

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

        x = (np.linalg.norm(ba) * np.linalg.norm(bc))
        y = np.dot(ba, bc)
        if y == 0 and x == 0:
            cosine_angle = 0
        else:
            cosine_angle = y / x
            if cosine_angle > 1:
                cosine_angle = 1
            elif cosine_angle < 0:
                cosine_angle = 0
        #
        # cosine_angle = np.dot(ba, bc) / (np.linalg.norm(ba) * np.linalg.norm(bc))

        angle = np.degrees(np.arccos(cosine_angle))
        return angle
