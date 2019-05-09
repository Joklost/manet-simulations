import numpy as np
import math


# np.seterr(all='raise')


class Link:
    def __init__(self, lat1: str, lon1: str, lat2: str, lon2: str, rssi: str, timestamp: str, node1_id: str,
                 node2_id: str, remove_distance: bool):
        self.timestamp = float(timestamp)
        self.node1_id = int(node1_id)
        self.node2_id = int(node2_id)
        self.node1 = [float(lat1), float(lon1)]
        self.node2 = [float(lat2), float(lon2)]
        self.distance = self._distance_between() * 1000
        self.rssi = (26 - Link.l_d(self.distance)) - float(rssi) if remove_distance else float(rssi)

    def __eq__(self, other):
        return ((self.node1[0] == other.node1[0] and self.node1[1] == other.node1[1]
                 and self.node2[0] == other.node2[0] and self.node2[1] == other.node2[1])
                or (self.node2[0] == other.node1[0] and self.node2[1] == other.node1[1]
                    and self.node1[0] == other.node2[0] and self.node1[1] == other.node2[1]))

    def __repr__(self) -> str:
        return f'timestamp: {self.timestamp}, node1_id: {self.node1_id}, node1: {self.node1}, node2_id: {self.node2_id}, node2: {self.node2}, distance: {self.distance}, rssi: {self.rssi}'

    def has_common_node(self, other) -> bool:
        return (self.node1 == other.node1
                or self.node1 == other.node2
                or self.node2 == other.node1
                or self.node2 == other.node2)

    @staticmethod
    def l_d(distance) -> float:
        return 0.0 if distance == 0 else 25 * math.log10(distance) + 45
        # return 0.0 if self.distance == 0 else 25 * math.log10(self.distance) + 45.8

    def reverse_link(self, link) -> bool:
        return link.node1 == self.node2 and link.node2 == self.node1

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
        self.rssi_diff = abs(self.link1.rssi - self.link2.rssi)

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
