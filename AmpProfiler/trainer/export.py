import json, torch, argparse
from model import DilatedAmpNet
p = argparse.ArgumentParser()
p.add_argument('--ckpt', type=str, default='model.pt')
p.add_argument('--onnx', type=str, default='modelA.onnx')
p.add_argument('--profile', type=str, default='profile.json')
p.add_argument('--sample_rate', type=int, default=48000)
p.add_argument('--oversample_default', type=int, default=2, choices=[1,2,4])
p.add_argument('--oversample_allowed', type=int, nargs='+', default=[1,2,4])
args = p.parse_args()

ckpt = torch.load(args.ckpt, map_location='cpu')
net = DilatedAmpNet(**ckpt.get('cfg', {})); net.load_state_dict(ckpt['state_dict']); net.eval()
dummy = torch.randn(1, 32768)
torch.onnx.export(net, dummy, args.onnx, input_names=['x'], output_names=['y'],
                  dynamic_axes={'x': {1: 'T'}, 'y': {1: 'T'}}, opset_version=17)

profile = {'sample_rate': args.sample_rate, 'oversample_default': args.oversample_default,
           'oversample_allowed': args.oversample_allowed}
with open(args.profile, 'w') as f: json.dump(profile, f, indent=2)
print('Exported', args.onnx, 'and', args.profile)
