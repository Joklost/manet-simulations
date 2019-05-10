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
        self.angle = self._angle_between()
        self.rssi_diff = abs(self.link1.rssi - self.link2.rssi)

    def __eq__(self, other):
        return (self.link1 == other.link1 and self.link2 == other.link2) or (
                self.link1 == other.link2 and self.link2 == other.link1)

    def _angle_between(self):
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

        lat_org_rad = math.radians(origin[0])
        lon_org_rad = math.radians(origin[1])

        courses = []
        for lat, lon in [loc1, loc2]:
            lat_rad = math.radians(lat)
            lon_rad = math.radians(lon)

            val = math.atan2(math.sin(lon_org_rad - lon_rad) * math.cos(lat_rad),
                             math.cos(lat_org_rad) * math.sin(lat_rad) -
                             math.sin(lat_org_rad) * math.cos(lat_rad) *
                             math.cos(lon_org_rad - lon_rad))
            courses.append(math.fmod(val, 2 * math.pi))

        return math.degrees(math.acos(math.cos(courses[0]) *
                                      math.cos(courses[1]) +
                                      math.sin(courses[0]) *
                                      math.sin(courses[1])))
