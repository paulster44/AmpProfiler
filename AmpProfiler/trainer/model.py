import torch, torch.nn as nn
class DilatedAmpNet(nn.Module):
    def __init__(self, channels=32, layers=8, kernel=3):
        super().__init__()
        dils = [2**i for i in range(layers)]; net=[]; in_ch=1
        for d in dils:
            net += [nn.Conv1d(in_ch, channels, kernel, padding=d, dilation=d), nn.LeakyReLU(0.2, inplace=True)]
            in_ch = channels
        net += [nn.Conv1d(in_ch, 1, 1)]
        self.net = nn.Sequential(*net)
    def forward(self, x):
        x = x.unsqueeze(1); y = self.net(x).squeeze(1); return y
