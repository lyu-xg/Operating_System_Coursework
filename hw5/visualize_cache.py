from numpy import *
import pylab as pl
import sys

from visualize_mem import *

units = {'KB': 2**10, 'MB': 2**20, 'GB': 2**30}
col_names = ['8KB', '16KB', '32KB', '64KB', '128KB', '256KB', '512KB', '1MB', '2MB', '4MB', '8MB', '16MB', '32MB', '64MB']

def size_to_num(s):
	if len(s) <= 2 or s[-1:] != 'B':
		return -1
	return (int(s[:-2]) * units[s[-2:]]) if s[-2:] in units else int(s[:-1])

# if True, all figures will be saved as PNG
SAVE = 1

def arraysize_stride(X, stride):
	X = array(X)
	stride = str(stride)

	# get the corresponding row as an array
	x = X[where(X.T[0]==stride)].reshape(-1)[1:-1]
	x = array([[float(t), col_names[i]] for i,t in enumerate(x) if t!='inf'])

	pl.figure(figsize=(15,12))
	xn = range(len(x.T[1]))
	pl.plot(xn, x.T[0])
	pl.xticks(xn, x.T[1])
	pl.xlabel('stride')
	pl.ylabel('runtime (seconds)')
	pl.savefig('arraysize_stride') if SAVE else pl.show()


def time_stride(X):
	X = array(X)

	X = X.T[:-1]
	N, D = X.T.shape

	xn = range(len(X[0]))

	pl.figure(figsize=(24,12))
	for d in range(1, D):

		inf_i = X[d].tolist().index('inf') if 'inf' in X[d] else -1
		
		# x = ['s-'+ x for x in X[0][:inf_i]]
		pl.plot(xn[:inf_i], [float(x) for x in X[d][:inf_i]])

	pl.xticks(xn, clean_units(X[0]))
	pl.legend(col_names, loc='upper left')

	pl.xlabel('stride')
	pl.ylabel('time')
	pl.savefig('time_stride') if SAVE else pl.show()

if __name__ == '__main__':

	IN = get_list(open(sys.argv[1])) if len(sys.argv) > 1 else 0

	if not IN:
		print("no data");
		exit(1);

	if len(sys.argv) > 2 and size_to_num(sys.argv[2])>0:
		arraysize_stride(IN, size_to_num(sys.argv[2]))
	else:
		time_stride(IN)