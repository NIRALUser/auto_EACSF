#!/tools/Python/Python-2.7.3/bin/python2.7
##	by Han Bit Yoon (email: hanbit.yoon@gmail.com)
#########################################################################################################
from __future__ import print_function
import argparse
import os
import sys
import subprocess
from subprocess import call

def main(args):
        Coronal_70Mask = args.t1
        PRE_VENT_MASK = args.ventricleMask
        PRE_CEREBELLUM_MASK = args.cerebellumMask
        Segmentation = args.tissueSeg
        OUT_PATH = args.output

        if (PRE_CEREBELLUM_MASK == ""):
               print ("Running rigid_align_script")
               call(["python", "rigid_align_script.py"])
               PRE_CEREBELLUM_MASK = './stx_noscale_718312_V24_t1w_RAI_Bias.nrrd'
               print ("Finished running rigid_align_script")
        if (PRE_VENT_MASK == ""):
               print ("Running make_mask_script")
               call(["python", "make_mask_script.py"])
               PRE_VENT_MASK = './stx_noscale_718312_V24_t1w_RAI_Bias_FinalMask.nrrd'
               print ("Finished running make_mask_script")
        if (Segmentation == ""):
               print ("Running vent_mask_script")
               call(["python", "vent_mask_script.py"])
               Segmentation = './stx_noscale_718312_V24_t1w_RAI_FINAL_Seg_withoutVent.nrrd'
               print ("Finished running vent_mask_script")

        Coronal_70Mask_dir = os.path.dirname(Coronal_70Mask)
        Coronal_70Mask_base = os.path.splitext(os.path.basename(Coronal_70Mask))[0]
        PRE_VENT_MASK_dir = os.path.dirname(PRE_VENT_MASK)
        PRE_VENT_MASK_base = os.path.splitext(os.path.basename(PRE_VENT_MASK))[0]
        PRE_CEREBELLUM_MASK_dir = os.path.dirname(PRE_CEREBELLUM_MASK)
        PRE_CEREBELLUM_MASK_base = os.path.splitext(os.path.basename(PRE_CEREBELLUM_MASK))[0]
        Segmentation_dir = os.path.dirname(Segmentation)
        Segmentation_base = os.path.splitext(os.path.basename(Segmentation))[0]

        ######### Cutting lateral ventricle ###00###
        MID_TEMP00 = os.path.join(OUT_PATH, "".join([Coronal_70Mask_base,"_MID00.nrrd"]))
        args=['ImageMath', Segmentation, '-outfile', MID_TEMP00, '-mul', PRE_VENT_MASK]
        print(" ".join(args))
        imgmath = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = imgmath.communicate()
        print(out)
        if(err):
                print(err, file=sys.stderr)

        ######### Cutting below AC-PC line #######
        MID_TEMP01 = os.path.join(OUT_PATH,"".join([Segmentation_base,"_MID01.nrrd"]))
        MID_TEMP02 = os.path.join(OUT_PATH,"".join([Segmentation_base,"_MID02.nrrd"]))
        MID_TEMP03 = os.path.join(OUT_PATH,"".join([Segmentation_base,"_MID03.nrrd"]))

        args=['ImageMath', MID_TEMP00, '-outfile', MID_TEMP01, '-mul', Coronal_70Mask]
        imgmath = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = imgmath.communicate()

        args=['ImageMath', MID_TEMP01, '-outfile', MID_TEMP02, '-extractLabel', '3']
        imgmath = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = imgmath.communicate()

        args=['ImageMath', MID_TEMP02, '-outfile', MID_TEMP02, '-sub', PRE_CEREBELLUM_MASK]
        imgmath = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = imgmath.communicate()

        args=['ImageMath', MID_TEMP02, '-outfile', MID_TEMP02, '-threshold', '1,1']
        imgmath = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = imgmath.communicate()

        args=['ImageMath', MID_TEMP02, '-outfile', MID_TEMP03, '-conComp','1']
        imgmath = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = imgmath.communicate()

        ######### Make thin mask for preserving outter 2nd or 3rd fracrtion csf ##########
        BRAIN_MASK = os.path.join(OUT_PATH,'Brain_Mask.nrrd')
        BRAIN_MASK_ERODE = os.path.join(OUT_PATH,'Brain_Mask_Erode.nrrd')
        THIN_MASK = os.path.join(OUT_PATH,'Brain_Thin_Mask.nrrd')

        args=['ImageMath', Segmentation, '-threshold', '1,4', '-outfile', BRAIN_MASK]
        imgmath = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = imgmath.communicate()

        args=['ImageMath', BRAIN_MASK, '-erode', '6,1', '-outfile', BRAIN_MASK_ERODE]
        imgmath = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = imgmath.communicate()

        args=['ImageMath', BRAIN_MASK, '-sub', BRAIN_MASK_ERODE, '-outfile', THIN_MASK]
        imgmath = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = imgmath.communicate()

        args=['ImageMath', THIN_MASK, '-mul', Coronal_70Mask, '-outfile', THIN_MASK]
        imgmath = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = imgmath.communicate()

        ########## Find outer CSF in thin mask and added this csf to extra CSF #############
        PRESERVED_OUTERCSF = os.path.join(OUT_PATH,'Preserved_OuterCSF.nrrd')
        FINAL_OUTERCSF = os.path.join(OUT_PATH, "".join([Segmentation_base,"_extCSF.nrrd"]))
        FINAL_OUTERCSF_dir = os.path.dirname(FINAL_OUTERCSF)
        FINAL_OUTERCSF_base = os.path.splitext(os.path.basename(FINAL_OUTERCSF))[0]

        args=['ImageMath', MID_TEMP02, '-outfile', PRESERVED_OUTERCSF, '-mul', THIN_MASK]
        imgmath = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = imgmath.communicate()
        print(out)
        if(err):
                print(err, file=sys.stderr)

        args=['ImageMath', MID_TEMP03, '-outfile', FINAL_OUTERCSF, '-add', PRESERVED_OUTERCSF]
        imgmath = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = imgmath.communicate()
        print(out)
        if(err):
                print(err, file=sys.stderr)

        args=['ImageMath', FINAL_OUTERCSF, '-outfile', FINAL_OUTERCSF, '-threshold', '1,2']
        imgmath = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = imgmath.communicate()
        print(out)
        if(err):
                print(err, file=sys.stderr)

        args=['ImageStat', FINAL_OUTERCSF, '-label', FINAL_OUTERCSF, '-outbase',
        os.path.join( FINAL_OUTERCSF_dir, "".join([FINAL_OUTERCSF_base,"_volume.txt"]))]
        imgstat = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = imgstat.communicate()
        print(out)
        if(err):
                print(err, file=sys.stderr)

        print("Auto_EACSF finished")
        sys.exit(0);

if (__name__ == "__main__"):
        parser = argparse.ArgumentParser(description='Calculates outter CSF')
        parser.add_argument('--t1', type=str, help='T1 Image', default="@T1IMG@")
        parser.add_argument('--ventricleMask', type=str, help='Ventricle mask', default="@VENTRICLE_MASK@")
        parser.add_argument('--cerebellumMask', type=str, help='Cereb Mask', default="@CEREB_MASK@")
        parser.add_argument('--tissueSeg', type=str, help='Tissue Segmentation', default="@TISSUE_SEG@")
        parser.add_argument('--output', type=str, help='Output directory', default="@OUTPUT_DIR@")
        args = parser.parse_args()

        main(args)
