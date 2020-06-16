#!/tools/Python/Python-3.6.2/bin/python3
##	by Han Bit Yoon, Arthur Le Maout, Martin Styner
#########################################################################################################
from __future__ import print_function
import argparse
import os
import sys
import subprocess
from subprocess import call
import itk
import numpy as np
import re

def call_and_print(args):
    #external process calling function with output and errors printing
    exe_path = args[0]
    print(" ".join(args))
    
    proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    
    out=out.decode('utf-8')
    err=err.decode('utf-8')
    if(out!=''):
        print(out)
    if(err!='' and err !='.'):
        print(err, file=sys.stderr)
    else:
        print('exit with success')

def find_file(pattern, directory):
    filename = None
    regex = re.compile(pattern)
    for root, dirs, files in os.walk(directory):
        for name in files:
            if regex.match(name) is not None:
                filename = os.path.join(root, name)
                break
    return filename


def main(main_args):
    print(os.path.basename(sys.argv[0])+"\n")
    ### Inputs
    T1 = main_args.t1
    T2 = main_args.t2
    BRAIN_MASK = main_args.brainMask
    T2_exists = True
    if (T2 == ""):
        T2_exists = False

    T1_base = os.path.splitext(os.path.splitext(os.path.basename(T1))[0])[0]

    if (T2_exists):
        T2_base = os.path.splitext(os.path.splitext(T2)[0])[0]

    SEGMENTATION = main_args.tissueSeg

    ### Executables
    python = main_args.python3
    ImageMath = main_args.ImageMath
    ImageStat = main_args.ImageStat
    ABC = main_args.ABC

    ### Masks
    CSFLabel = int(main_args.CSFLabel)

    ### Output path
    OUT_PATH = main_args.output

    ### Scripts
    scripts_prefix = os.path.join(OUT_PATH, "PythonScripts")


    current_suffix = ""

    registration_suffix = "_stx"
    if (main_args.registration):
        rigid_align_script = os.path.join(scripts_prefix, "rigid_align.py")
        print("Running " + rigid_align_script)
        call_and_print([python, rigid_align_script])

    if (main_args.skullStripping):
        make_mask_script = os.path.join(scripts_prefix, "make_mask.py")
        print("Running " + make_mask_script)
        call_and_print([python, make_mask_script])

    skull_strip_script = os.path.join(scripts_prefix, "skull_strip.py")
    print("Running " + skull_strip_script)
    call_and_print([python, skull_strip_script])

    if (main_args.segmentation):
        tissue_seg_script = os.path.join(scripts_prefix, "tissue_seg.py")
        print("Running " + tissue_seg_script)
        call_and_print([python, tissue_seg_script])
        abc_seg = find_file(".*_labels_EMS.nrrd", os.path.join(OUT_PATH, "ABC_Segmentation"))
        if(abc_seg is not None):
            SEGMENTATION = abc_seg


    if (main_args.removeVentricles):
        vent_mask_script = os.path.join(scripts_prefix, "vent_mask.py")
        print("Running " + vent_mask_script)
        call_and_print([python, vent_mask_script])
        
        ventricleMasking_seg = find_file(".*_withoutVent.nrrd", os.path.join(OUT_PATH, "VentricleMasking"))
        if(ventricleMasking_seg is not None):
            SEGMENTATION = ventricleMasking_seg
        print("Finished running "+ vent_mask_script)

    OUT_FM = os.path.join(OUT_PATH, 'FinalMasking')
    if not os.path.exists(OUT_FM):
        os.makedirs(OUT_FM)

    if(not os.path.exists(BRAIN_MASK)):
        BRAIN_MASK = find_file(".*_FinalBrainMask.nrrd", os.path.join(OUT_PATH, "SkullStripping"))
        # BRAIN_MASK = os.path.join(OUT_PATH, 'FinalMasking', os.path.splitext(os.path.basename(SEGMENTATION))[0] + "_brainMask.nrrd")
        # args=[ImageMath, SEGMENTATION, '-outfile', BRAIN_MASK, '-threshold', "1,999999"]
        # call_and_print(args)

    BRAIN_MASK_base = os.path.splitext(os.path.basename(BRAIN_MASK))[0]
    Segmentation_base = os.path.splitext(os.path.basename(SEGMENTATION))[0]

    ######### Stripping the skull from segmentation ######
    MID_TEMP00 = os.path.join(OUT_FM, "".join([T1_base,"_MID00.nrrd"]))
    args=[ImageMath, SEGMENTATION, '-outfile', MID_TEMP00, '-mask', BRAIN_MASK]
    call_and_print(args)

    ######### Cutting below AC-PC line #######
    ### Coronal mask creation
    print("Cutting below AC-PC line")

    T1_REGISTERED = T1
    if main_args.registration:
        T1_REGISTERED = os.path.join(OUT_PATH, "RigidRegistration", T1_base + "_stx.nrrd")
        T1_base = os.path.splitext(os.path.splitext(os.path.basename(T1_REGISTERED))[0])[0]

    
    T1_STRIPPED = os.path.join(OUT_PATH, "SkullStripping", T1_base + "_stripped.nrrd")
    if(T1_STRIPPED is not None):
        T1_REGISTERED = T1_STRIPPED
        T1_base = os.path.splitext(os.path.splitext(os.path.basename(T1_REGISTERED))[0])[0]

    if (main_args.cerebellumMask == ""):
        ACPC_unit=main_args.ACPCunit
        if(ACPC_unit == "index"):
            ACPC_val=int(main_args.ACPCval)
        else:
            ACPC_mm=float(main_args.ACPCval)
            im=itk.imread(T1_REGISTERED)
            index_coord=im.TransformPhysicalPointToContinuousIndex([ACPC_mm,0,0])
            ACPC_val=round(index_coord[0])

        Coronal_Mask = os.path.join(OUT_FM,"coronal_mask_"+str(ACPC_val)+".nrrd")
        if not (os.path.isfile(Coronal_Mask)):
            im=itk.imread(T1_REGISTERED)
            np_copy=itk.GetArrayFromImage(im)
            if ((ACPC_val >= np_copy.shape[0]) | (ACPC_val <= 0)):
                eprint("ACPC index out of range ("+str(ACPC_val)+"), using default coronal mask (slice 70)")
                sys.stderr.flush()
                ACPC_val = 70;

            print('Creating coronal mask')
            np_copy[ACPC_val-1:np_copy.shape[0]-1,:,:]=1
            np_copy[0:ACPC_val-1,:,:]=0
            itk_np_copy=itk.GetImageViewFromArray(np_copy)
            itk_np_copy.SetOrigin(im.GetOrigin())
            itk_np_copy.SetSpacing(im.GetSpacing())
            itk_np_copy.SetDirection(im.GetDirection())
            itk.imwrite(itk_np_copy,Coronal_Mask)
            print('Coronal mask created')
        else:
            print('Loading ' + Coronal_Mask)

    else:
        Coronal_Mask = main_args.cerebellumMask

    ### Mask multiplication
    MID_TEMP01 = os.path.join(OUT_FM,"".join([Segmentation_base,"_MID01.nrrd"]))
    MID_TEMP02 = os.path.join(OUT_FM,"".join([Segmentation_base,"_MID02.nrrd"]))
    MID_TEMP03 = os.path.join(OUT_FM,"".join([Segmentation_base,"_MID03.nrrd"]))

    args=[ImageMath, MID_TEMP00, '-outfile', MID_TEMP01, '-mask', Coronal_Mask]
    call_and_print(args)

    args=[ImageMath, MID_TEMP01, '-outfile', MID_TEMP02, '-extractLabel', '3']
    call_and_print(args)

    args=[ImageMath, MID_TEMP02, '-outfile', MID_TEMP03, '-conComp','1']
    call_and_print(args)

    Erosion_Mask = os.path.join(OUT_FM,BRAIN_MASK_base + '_Erosion.nrrd')
    Erosion_Mask00 = os.path.join(OUT_FM,BRAIN_MASK_base + '_Erosion00.nrrd')
    Erosion_Mask01 = os.path.join(OUT_FM,BRAIN_MASK_base + '_Erosion01.nrrd')
    Erosion_Mask01_inv = os.path.join(OUT_FM,BRAIN_MASK_base + '_Erosion01_inv.nrrd')
    args = [ImageMath, BRAIN_MASK, '-erode', '1,1', '-outfile', Erosion_Mask00]
    call_and_print(args)

    args = [ImageMath, Erosion_Mask00, '-erode', '1,1', '-outfile', Erosion_Mask01]
    call_and_print(args)

    for i in range(0,14):
        args = [ImageMath, Erosion_Mask01,  '-erode', '1,1', '-outfile', Erosion_Mask01]
        call_and_print(args)

    args = [ImageMath, Erosion_Mask01, '-threshold', '0,0', '-outfile', Erosion_Mask01_inv]
    call_and_print(args)


    FINAL_CSF_SEG = os.path.join(OUT_FM, Segmentation_base + "_Partial_CSF.nrrd")

    args = [ImageMath, MID_TEMP03, '-mask', Erosion_Mask01_inv, '-outfile', FINAL_CSF_SEG]
    call_and_print(args)

    ## add script erase quad....
    Erosion = os.path.join(OUT_FM, Segmentation_base + "Partial_Ero.nrrd")
    Erosion_MASKING = os.path.join(OUT_FM,Segmentation_base + "Partial_Masking.nrrd")
    args = [ImageMath, FINAL_CSF_SEG,  '-erode', '1,1', '-outfile', Erosion]
    call_and_print(args)

    args = [ImageMath, Erosion_Mask01,  '-erode', '1,1', '-outfile', Erosion_Mask]
    call_and_print(args)
    for i in range(0,5):
        args = [ImageMath, Erosion_Mask,  '-erode', '1,1', '-outfile', Erosion_Mask]
        call_and_print(args)

    args = [ImageMath, Erosion, '-mask', Erosion_Mask, '-outfile', Erosion_MASKING]
    call_and_print(args)


    Erosion_MASKING_conComp = os.path.join(OUT_FM, Segmentation_base + "Partial_conComp.nrrd")
    args = [ImageMath, Erosion_MASKING, '-conComp', '1', '-outfile', Erosion_MASKING_conComp]
    call_and_print(args)


    Dilation_comComp = os.path.join(OUT_FM, Segmentation_base + "Partial_conComp_dil.nrrd")
    args = [ImageMath, Erosion_MASKING_conComp,  '-dilate', '1,1', '-outfile', Dilation_comComp]
    call_and_print(args)

    for k in range(0,13):
        args = [ImageMath, Dilation_comComp,  '-dilate', '1,1', '-outfile', Dilation_comComp]
        call_and_print(args)

    FINAL_RESULT = os.path.join(OUT_PATH, Segmentation_base + "_FINAL_QCistern.nrrd")

    args = [ImageMath, Dilation_comComp, '-threshold', '0,0', '-outfile', Dilation_comComp]
    call_and_print(args)
    args = [ImageMath, FINAL_CSF_SEG, '-mask', Dilation_comComp, '-outfile', FINAL_RESULT]
    call_and_print(args)

    args = [ImageStat, FINAL_RESULT, '-label', FINAL_RESULT, '-volumeSummary']
    call_and_print(args)

    if (main_args.computeCSFDensity == "true"):
        LH_INNER_SURF = main_args.LHInnerSurf
        RH_INNER_SURF = main_args.RHInnerSurf
        OUT_CCD=os.path.join(OUT_PATH,'CSF_Density')
        args = ['computecsfdensity', '-l', LH_INNER_SURF, '-r', RH_INNER_SURF, '-s', Segmentation, '-c', FINAL_OUTERCSF, '-o', OUT_CCD, '-p', Segmentation_base]
        call_and_print(args)

    print("Auto_EACSF finished")

    sys.exit(0);

if (__name__ == "__main__"):
    parser = argparse.ArgumentParser(description='Calculates outter CSF')
    parser.add_argument('--t1', type=str, help='T1 Image', default="@T1IMG@")
    parser.add_argument('--t2', type=str, help='T2 Image', default="@T2IMG@")
    parser.add_argument('--brainMask', type=str, help='Brain mask', default="@BRAIN_MASK@")
    parser.add_argument('--cerebellumMask', type=str, help='Cereb Mask', default="@CEREB_MASK@")
    parser.add_argument('--tissueSeg', type=str, help='Tissue Segmentation', default="@TISSUE_SEG@")
    parser.add_argument('--CSFLabel', type=int, help='CSF Label in segmentation', default=@CSF_LABEL@)
    parser.add_argument('--ACPCunit', type=str, help='ACPC unit (mm/index)', default="@ACPC_UNIT@")
    parser.add_argument('--ACPCval', type=float, help='ACPC value', default=@ACPC_VAL@)
    parser.add_argument('--LHInnerSurf', type=str, help='Left hemisphere inner surface', default="@LH_INNER@")
    parser.add_argument('--RHInnerSurf', type=str, help='Right hemisphere inner surface', default="@RH_INNER@")
    parser.add_argument('--registration', type=bool, help='Perform rigid registration', default=@PERFORM_REG@)
    parser.add_argument('--skullStripping', type=bool, help='Perform skull stripping', default=@PERFORM_SS@)
    parser.add_argument('--segmentation', type=bool, help='Perform tissue segmentation', default=@PERFORM_TSEG@)
    parser.add_argument('--removeVentricles', type=bool, help='Perform tissue segmentation', default=@PERFORM_VR@)
    parser.add_argument('--computeCSFDensity', type=bool, help='Compute CSF density', default=@COMPUTE_CSFDENS@)
    parser.add_argument('--python3', type=str, help='Python3 executable path', default='@python3_PATH@')
    parser.add_argument('--ImageMath', type=str, help='ImageMath executable path', default='@ImageMath_PATH@')
    parser.add_argument('--ImageStat', type=str, help='ImageStat executable path', default='@ImageStat_PATH@')
    parser.add_argument('--ABC', type=str, help='ABC executable path', default='@ABC_CLI_PATH@')
    parser.add_argument('--output', type=str, help='Output directory', default="@OUTPUT_DIR@")
    args = parser.parse_args()
    main(args)
