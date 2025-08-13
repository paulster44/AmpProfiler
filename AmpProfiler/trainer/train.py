import argparse, torch
from torch.utils.data import DataLoader
from dataset import PairDataset
from model import DilatedAmpNet
from losses import full_loss

p = argparse.ArgumentParser()
p.add_argument('--di', nargs='+', required=True)
p.add_argument('--amp', nargs='+', required=True)
p.add_argument('--epochs', type=int, default=20)
p.add_argument('--chunk', type=int, default=32768)
p.add_argument('--batch', type=int, default=8)
p.add_argument('--lr', type=float, default=2e-4)
p.add_argument('--channels', type=int, default=32)
p.add_argument('--layers', type=int, default=8)
p.add_argument('--out', type=str, default='model.pt')
args = p.parse_args()

ds = PairDataset(args.di, args.amp, chunk=args.chunk)
ld = DataLoader(ds, batch_size=args.batch, shuffle=True, num_workers=0, drop_last=True)
dev = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
net = DilatedAmpNet(channels=args.channels, layers=args.layers).to(dev)
opt = torch.optim.Adam(net.parameters(), lr=args.lr)

for e in range(1, args.epochs+1):
    net.train()
    for xb, yb in ld:
        xb = xb.to(dev); yb = yb.to(dev)
        pred = net(xb); loss = full_loss(pred, yb)
        opt.zero_grad(); loss.backward(); opt.step()
    print(f"epoch {e}/{args.epochs} loss {loss.item():.5f}")

torch.save({'state_dict': net.state_dict(), 'cfg': {'channels': args.channels, 'layers': args.layers}}, args.out)
print('Saved', args.out)
