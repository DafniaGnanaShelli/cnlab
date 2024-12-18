
from mininet.topo import Topo

class Mesh(Topo):
    def build(self, n=4):
        switches = []
        
        # Add switches
        for i in range(n):
            switch = self.addSwitch('s{}'.format(i + 1))
            switches.append(switch)
        
        # Add hosts and connect them to the switches
        for i in range(n):
            host = self.addHost('h{}'.format(i + 1))
            self.addLink(host, switches[i])
            
        # Connect each switch to every other switch (mesh)
        for i in range(n):
            for j in range(i + 1, n):
                self.addLink(switches[i], switches[j])

topos = {'mesh': Mesh}
