#!/tools/Python/Python-3.6.2/bin/python3
##	by Arthur Le Maout (alemaout@email.unc.edu)
#########################################################################################################

import sys
import os
import argparse
import subprocess
from main_script import eprint
from main_script import call_and_print

def main(args):
    T1 = args.t1
    T2 = args.t2
    ATLAS_DIR = args.at_dir
    BRAINSFit = args.BRAINSFit
    ANTS = args.ANTS
    WarpImageMultiTransform = args.WarpImageMultiTransform
    ABC=args.ABC
    OUTPUT_DIR = args.output

    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)
    PARENT_DIR = os.path.dirname(OUTPUT_DIR)

    TEMPLATE = os.path.join(ATLAS_DIR,'template.mha')
    AFFINE_TF = os.path.join(OUTPUT_DIR,'output_affine_transform.txt')
    TEMPLATE_AFF = os.path.join(OUTPUT_DIR,'template_affine.nrrd')
    ANTs_MATRIX_NAME = os.path.join(OUTPUT_DIR,'template_to_T1_')
    ANTs_WARP = ''.join([ANTs_MATRIX_NAME,'Warp.nii.gz'])
    ANTs_INV_WARP =  ''.join([ANTs_MATRIX_NAME,'InverseWarp.nii.gz'])
    ANTs_AFFINE = ''.join([ANTs_MATRIX_NAME,'Affine.txt'])
    TEMPLATE_FIN = os.path.join(OUTPUT_DIR,'template.mha')

    if not (os.path.isfile(TEMPLATE) and os.path.isfile(AFFINE_TF) and os.path.isfile(TEMPLATE_AFF) and os.path.isfile(ANTs_WARP) and os.path.isfile(ANTs_INV_WARP) and os.path.isfile(ANTs_AFFINE) and os.path.isfile(TEMPLATE_FIN)):
        args = [BRAINSFit, '--fixedVolume', T1, '--movingVolume', TEMPLATE, '--outputTransform',AFFINE_TF, '--useRigid', '--initializeTransformMode', 'useCenterOfHeadAlign']
        call_and_print(args)

        args = [WarpImageMultiTransform, '3', TEMPLATE, TEMPLATE_AFF, AFFINE_TF, '-R', T1]
        call_and_print(args)

        args = [ANTS, '3', '-m', 'CC['+T1+','+TEMPLATE+',1,4]', '-i', '100x50x25', '-o', ANTs_MATRIX_NAME, '-t', 'SyN[0.25]', '-r', 'Gauss[3,0]']
        call_and_print(args)

        args = [WarpImageMultiTransform, '3', TEMPLATE, TEMPLATE_FIN, ANTs_WARP, ANTs_AFFINE, '-R', T1]
        call_and_print(args)
    else:
        print_aef('ANTs registration already exists')

    for i in range(1,6):
        FILE = os.path.join(ATLAS_DIR,str(i)+'.mha')
        FILE_FIN = os.path.join(OUTPUT_DIR,str(i)+'.mha')

        if not (os.path.isfile(FILE_FIN)):
            args = [WarpImageMultiTransform, '3', FILE, FILE_FIN, ANTs_WARP, ANTs_AFFINE, '-R', T1]
            call_and_print(args)
        else:
            print_aef(FILE_FIN+' already exist')

    args = [ABC, os.path.join(PARENT_DIR,'ABCparam.xml')]
    call_and_print(args)

if (__name__ == "__main__"):
    parser = argparse.ArgumentParser(description='Prepare the atlas and calls ABC for tissue segmentation')
    parser.add_argument('--t1', nargs='?', type=str, help='T1 Image to calculate deformation field against atlas', const="@T1IMG@")
    parser.add_argument('--t2', nargs='?', type=str, help='T2 Image to calculate deformation field against atlas', const="@T2IMG@")
    parser.add_argument('--at_dir', nargs='?', type=str, help='atlases directory', const="@ATLASES_DIR@")
    parser.add_argument('--BRAINSFit', type=str, help='BRAINSFit executable path', default="@BRAINSFit_PATH@")
    parser.add_argument('--ANTS', type=str, help='ANTS executable path', default='@ANTS_PATH@')
    parser.add_argument('--WarpImageMultiTransform', type=str, help='WarpImageMultiTransform executable path', default='@WarpImageMultiTransform_PATH@')
    parser.add_argument('--ABC', type=str, help='ABC executable path', default='@ABC_CLI_PATH@')
    parser.add_argument('--output', nargs='?', type=str, help='Output directory', const="@OUTPUT_DIR@")
    args = parser.parse_args()
    main(args)
