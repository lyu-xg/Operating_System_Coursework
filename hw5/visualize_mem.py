from numpy import *
import pylab as pl
import sys

def get_list(s):
	return [l.split('\t') for l in s.read().split('\n') if '\t' in l]

def clean_unit(s):
	s = int(s)
	if s >= 1073741824:
		return str(s/1073741824) + 'GB'
	if s >= 1048576:
		return str(s/1048576) + 'MB'
	elif s >= 1024:
		return str(s/1024) + 'KB'
	else:
		return str(s) + 'B'

def clean_units(S):
	return [clean_unit(s) for s in S]

def selective(S, k):
	# make elements in S visible every k element
	return [(s if not i%k else '') for i,s in enumerate(S)]

def selective_clean(S, k):
	return selective(clean_units(S), k)


if __name__ == "__main__":

	IN = get_list(open(sys.argv[1])) if len(sys.argv) > 1 else 0

	if not IN:
		print("no data");
		exit(1);

	X = array(IN)

	xn = range(len(X.T[0]))

	pl.plot(xn, [float(x)*1000 for x in X.T[1]])

	pl.xticks(xn, selective_clean(X.T[0], 10))
	pl.xlabel('size of requested memory (bytes)')
	pl.ylabel('time took to touch all requested pages (ms)')
	pl.show()