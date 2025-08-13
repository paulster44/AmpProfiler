import random, numpy as np, soundfile as sf, torch
from torch.utils.data import Dataset
class PairDataset(Dataset):
    def __init__(self, di_files, amp_files, chunk=32768, gain_norm=True):
        assert len(di_files)==len(amp_files)>0
        self.x, self.y = [], []
        for d,a in zip(di_files, amp_files):
            x,srx = sf.read(d, always_2d=False); y,sry = sf.read(a, always_2d=False)
            assert srx==sry
            x = np.asarray(x, dtype=np.float32); y = np.asarray(y, dtype=np.float32)
            n = min(len(x), len(y)); x, y = x[:n], y[:n]
            if gain_norm:
                x /= (np.max(np.abs(x))+1e-8); y /= (np.max(np.abs(y))+1e-8)
            self.x.append(x); self.y.append(y)
        self.chunk = int(chunk)
    def __len__(self): return 100000
    def __getitem__(self, _):
        import numpy as np
        i = random.randrange(len(self.x)); x,y = self.x[i], self.y[i]; n = len(x)
        if n <= self.chunk:
            pad = self.chunk - n + 1; x = np.pad(x,(0,pad)); y = np.pad(y,(0,pad)); n = len(x)
        s = random.randrange(0, n-self.chunk)
        xb = x[s:s+self.chunk]; yb = y[s:s+self.chunk]
        return torch.from_numpy(xb).float(), torch.from_numpy(yb).float()
