#!/tools/Python/Python-3.6.2/bin/python3
##	by Arthur Le Maout (alemaout@email.unc.edu)
#########################################################################################################

import sys
import os
import argparse
import subprocess
import re

from main_script import call_and_print

def find_files(pattern, directory):
    filenames = []
    regex = re.compile(pattern)
    for root, dirs, files in os.walk(directory):
        for name in files:
            if regex.match(name) is not None:
                filenames.append(os.path.join(root, name))
    return filenames


def main(args):

    ABC_TEMPLATE="<?xml version=\"1.0\"?>\n\
<!DOCTYPE SEGMENTATION-PARAMETERS>\n\
<SEGMENTATION-PARAMETERS>\n\
<SUFFIX>EMS</SUFFIX>\n\
<ATLAS-DIRECTORY>@ATLAS_DIR@</ATLAS-DIRECTORY>\n\
<ATLAS-ORIENTATION>file</ATLAS-ORIENTATION>\n\
<OUTPUT-DIRECTORY>@OUTPUT_DIR@</OUTPUT-DIRECTORY>\n\
<OUTPUT-FORMAT>Nrrd</OUTPUT-FORMAT>\n\
<IMAGE>\n\
\t<FILE>@T1_INSEG_IMG@</FILE>\n\
\t<ORIENTATION>file</ORIENTATION>\n\
</IMAGE>\n\
<IMAGE>\n\
\t<FILE>@T2_INSEG_IMG@</FILE>\n\
\t<ORIENTATION>file</ORIENTATION>\n\
</IMAGE>\n\
<FILTER-ITERATIONS>10</FILTER-ITERATIONS>\n\
<FILTER-TIME-STEP>0.01</FILTER-TIME-STEP>\n\
<FILTER-METHOD>Curvature flow</FILTER-METHOD>\n\
<MAX-BIAS-DEGREE>4</MAX-BIAS-DEGREE>\n\
<PRIOR>1.3</PRIOR>\n\
<PRIOR>1</PRIOR>\n\
<PRIOR>0.7</PRIOR>\n\
@EXTRA_PRIOR@\n\
<PRIOR>0.8</PRIOR>\n\
<INITIAL-DISTRIBUTION-ESTIMATOR>robust</INITIAL-DISTRIBUTION-ESTIMATOR>\n\
<DO-ATLAS-WARP>0</DO-ATLAS-WARP>\n\
<ATLAS-WARP-FLUID-ITERATIONS>50</ATLAS-WARP-FLUID-ITERATIONS>\n\
<ATLAS-WARP-FLUID-MAX-STEP>0.1</ATLAS-WARP-FLUID-MAX-STEP>\n\
<ATLAS-LINEAR-MAP-TYPE>id</ATLAS-LINEAR-MAP-TYPE>\n\
<IMAGE-LINEAR-MAP-TYPE>id</IMAGE-LINEAR-MAP-TYPE>\n\
</SEGMENTATION-PARAMETERS>"

    T1 = args.t1
    T2 = args.t2
    ATLAS_DIR = args.at_dir
    BRAINSFit = args.BRAINSFit
    ANTS = args.ANTS
    WarpImageMultiTransform = args.WarpImageMultiTransform
    ABC=args.ABC
    OUTPUT_DIR = args.output
    OUTPUT_DIR_TISSUE_ATLAS = os.path.join(OUTPUT_DIR, "TissueSegmentationAtlas")

    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)

    if not os.path.exists(OUTPUT_DIR_TISSUE_ATLAS):
        os.makedirs(OUTPUT_DIR_TISSUE_ATLAS)

    TEMPLATE = os.path.join(ATLAS_DIR,'template.mha')
    AFFINE_TF = os.path.join(OUTPUT_DIR_TISSUE_ATLAS,'output_affine_transform.txt')
    TEMPLATE_AFF = os.path.join(OUTPUT_DIR_TISSUE_ATLAS,'template_affine.nrrd')
    ANTs_MATRIX_NAME = os.path.join(OUTPUT_DIR_TISSUE_ATLAS,'template_to_T1_')
    ANTs_WARP = ''.join([ANTs_MATRIX_NAME,'Warp.nii.gz'])
    ANTs_INV_WARP =  ''.join([ANTs_MATRIX_NAME,'InverseWarp.nii.gz'])
    ANTs_AFFINE = ''.join([ANTs_MATRIX_NAME,'Affine.txt'])
    TEMPLATE_FIN = os.path.join(OUTPUT_DIR_TISSUE_ATLAS,'template.mha')

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
        print('ANTs registration already exists')

    predefined_priors=[1.3, 1.0, 0.7, 0.8]

    priors_found = find_files("\\d\\.mha", ATLAS_DIR)

    for pr_name in priors_found:
        
        FILE_FIN = os.path.join(OUTPUT_DIR_TISSUE_ATLAS,os.path.basename(pr_name))

        if not (os.path.isfile(FILE_FIN)):
            args = [WarpImageMultiTransform, '3', pr_name, FILE_FIN, ANTs_WARP, ANTs_AFFINE, '-R', T1]
            call_and_print(args)

    priors = []
    for i in range(len(priors_found) - 1):
        if i < len(predefined_priors) - 1:
            priors.append(predefined_priors[i])
        else:
            priors.append(1)

    priors.append(predefined_priors[-1])

    PRIORS = ""
    for pr in priors:
        PRIORS += "<PRIOR>" + str(pr) + "</PRIOR>\n"
    
    segfiledata = ABC_TEMPLATE.replace('@T1_INSEG_IMG@',T1)
    segfiledata = segfiledata.replace('@T2_INSEG_IMG@',T2)
    segfiledata = segfiledata.replace('@ATLAS_DIR@',ATLAS_DIR)
    segfiledata = segfiledata.replace('@PRIORS@',PRIORS)

    abc_params = os.path.join(OUTPUT_DIR,'ABCparam.xml')
    segfile = open(abc_params,'w')
    segfile.write(segfiledata)
    segfile.close()

    labels_out = os.path.join(OUTPUT_DIR, os.path.splitext(os.path.basename(T1))[0] + "_labels_EMS.nrrd")
    
    if not (os.path.isfile(labels_out)):
        args = [ABC, abc_params]
        call_and_print(args)

if (__name__ == "__main__"):
    parser = argparse.ArgumentParser(description='Prepare the atlas and calls ABC for tissue segmentation')
    parser.add_argument('--t1', type=str, help='T1 Image to calculate deformation field against atlas', default="@T1IMG@")
    parser.add_argument('--t2', type=str, help='T2 Image to calculate deformation field against atlas', default="@T2IMG@")
    parser.add_argument('--at_dir', type=str, help='atlases directory', default="@ATLASES_DIR@")
    parser.add_argument('--BRAINSFit', type=str, help='BRAINSFit executable path', default="@BRAINSFit_PATH@")
    parser.add_argument('--ANTS', type=str, help='ANTS executable path', default='@ANTS_PATH@')
    parser.add_argument('--WarpImageMultiTransform', type=str, help='WarpImageMultiTransform executable path', default='@WarpImageMultiTransform_PATH@')
    parser.add_argument('--ABC', type=str, help='ABC executable path', default='@ABC_CLI_PATH@')
    parser.add_argument('--output', type=str, help='Output directory', default="@OUTPUT_DIR@")
    args = parser.parse_args()
    main(args)
