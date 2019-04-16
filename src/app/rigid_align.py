#!/tools/Python/Python-3.6.2/bin/python3
##	by Han Bit Yoon, Arthur Le Maout (alemaout@email.unc.edu)
######################################################################################################### 

import sys
import os
import argparse
import subprocess
from main_script import eprint
from main_script import call_and_print
from main_script import print_aef

def main(args):
    sys.stdout.flush()
    T1 = args.t1
    T2 = args.t2
    ATLAS = args.atlas
    BRAINSFit=args.BRAINSFit
    OUTPUT_DIR = args.output
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)

    T2_exists=True
    if (T2 == ""):
        T2_exists=False

    T1_dir=os.path.dirname(T1)

    T1_split=os.path.splitext(os.path.basename(T1))
    if (T1_split[1] == 'gz'):
        T1_base=os.path.splitext(T1_split[0])
    else:
        T1_base=T1_split[0]

    STX_T1 = os.path.join(OUTPUT_DIR, "".join([T1_base,"_stx.nrrd"]))

    if (T2_exists):
        T2_dir=os.path.dirname(T2)
        T2_split=os.path.splitext(os.path.basename(T2))
        if (T2_split[1] == 'gz'):
            T2_base=os.path.splitext(T2_split[0])
        else:
            T2_base=T2_split[0]
        STX_T2 = os.path.join(OUTPUT_DIR, "".join([T2_base,"_stx.nrrd"]))

    if (not os.path.isfile(STX_T1)):
        args=[BRAINSFit, '--fixedVolume', ATLAS, '--movingVolume', T1, '--outputVolume', STX_T1, '--useRigid',\
        '--initializeTransformMode', 'useCenterOfHeadAlign', '--outputVolumePixelType', 'short']
        call_and_print(args)
    else:
        print_aef("T1 image already aligned")

    if (T2_exists):
        if (not os.path.isfile(STX_T2)):
            args=[BRAINSFit, '--fixedVolume', STX_T1, '--movingVolume', T2, '--outputVolume', STX_T2, '--useRigid',\
            '--initializeTransformMode', 'useCenterOfHeadAlign', '--outputVolumePixelType', 'short']
            call_and_print(args)
        else:
            print_aef("T2 image already aligned")

if (__name__ == "__main__"):
    parser = argparse.ArgumentParser(description='Calculates segmentation w/o ventricle mask. Computes deformation field with T1 vs ATLAS, applies warp to ventricle mask and masks tissue-seg')
    parser.add_argument('--t1', type=str, help='T1 Image to calculate deformation field against atlas', default="@T1IMG@")
    parser.add_argument('--t2', type=str, help='T2 Image to calculate deformation field against atlas', default="@T2IMG@")
    parser.add_argument('--atlas', type=str, help='Atlas image', default="@ATLAS@")
    parser.add_argument('--BRAINSFit', type=str, help='BRAINSFit executable path', default="@BRAINSFit_PATH@")
    parser.add_argument('--output', type=str, help='Output directory', default="@OUTPUT_DIR@")
    args = parser.parse_args()
    main(args)
