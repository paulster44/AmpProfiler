import torch, torch.nn.functional as F
def mrstft_loss(x, y):
    loss = 0.0
    for w in (256,512,1024):
        X = torch.stft(x, w, w//4, return_complex=True)
        Y = torch.stft(y, w, w//4, return_complex=True)
        loss += (X - Y).abs().mean()
    return loss
def full_loss(pred, target, alpha=0.5):
    return F.l1_loss(pred, target) + alpha*mrstft_loss(pred, target)
