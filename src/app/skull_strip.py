#!/tools/Python/Python-3.6.2/bin/python3
##	by Han Bit Yoon, Arthur Le Maout, Martin Styner
#########################################################################################################  
import sys
import os
import argparse
import subprocess
import json

from main_script import call_and_print

def main(args):

	T1 = args.t1
	T2 = args.t2
	BrainMask = args.mask
	output_dir = args.output
	ImageMath = args.ImageMath

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

	if(BrainMask is not None and os.path.exists(BrainMask)):
		print("Skull stripping ...")
		if(T1 is not None and os.path.exists(T1)):
			T1_Base = os.path.splitext(os.path.basename(T1))[0]
			T1_STRIPPED = os.path.join(output_dir, T1_Base + "_stripped.nrrd")
			args=[ImageMath, T1, '-mask', BrainMask, '-outfile', T1_STRIPPED]
			call_and_print(args)

		if(T2 is not None and os.path.exists(T1)):
			T2_Base = os.path.splitext(os.path.basename(T2))[0]
			T2_STRIPPED = os.path.join(output_dir, T2_Base + "_stripped.nrrd")
			args=[ImageMath, T2, '-mask', BrainMask, '-outfile', T2_STRIPPED]
			call_and_print(args)

if (__name__ == "__main__"):
    parser = argparse.ArgumentParser(description='Skull strip the images')
    parser.add_argument('--t1', type=str, help='T1 Image to calculate deformation field against atlas', default="@T1IMG@")
    parser.add_argument('--t2', type=str, help='T2 Image to calculate deformation field against atlas', default="@T2IMG@")
    parser.add_argument('--mask', type=str, help='mask the images', default="@BRAINMASKIMG@")
    parser.add_argument('--ImageMath', type=str, help='ImageMath executable path', default='@ImageMath_PATH@')
    parser.add_argument('--output', type=str, help='Output directory', default="@OUTPUT_DIR@")
    args = parser.parse_args()
    main(args)