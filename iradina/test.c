
/*********************************************************************

Copyright 2019, Christian Borschel

This file is part of iradina.

iradina is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

iradina is distributed WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with iradina.  If not, see <http://www.gnu.org/licenses/>.

***********************************************************************/

					    /**************************************************************************************************/
					    /* Kinchin Pease calculation of damage was added by Jean-Paul Crocombette (CROC) from CEA Saclay. */
					    /* Some modifications were introduced by Christian Van Wambeke (CVW) from CEA Saclay in order to  */
					    /* support the CEA GUI.                                                                           */
					    /**************************************************************************************************/


					    /********************************************************************/
					    /* This module contains some utility functions                      */
					    /********************************************************************/

					    /* Error numbers in this file: 4000-4999 */


#include "utils.h"


int make_double_array(char* values, int count, double* d_array){
  /* Read comma-seprated values from string and put them into
     the double array, which has #count entries. The double array
     must exist already */
  int i=1;
  char* temp;
	
  temp=(char*)malloc(sizeof(char)*32);
  temp=strtok(values,",");
  sscanf(temp,"%lg",&(d_array[0]));
  while(((temp=strtok(NULL,","))!=NULL)&&(i<count)){
    sscanf(temp,"%lg",&(d_array[i]));
    i++;
  }
  return 0;
}

int make_float_array(char* values, int count, float* f_array){
  /* Read comma-seprated values from string and put them into
     the float array, which has #count entries. The float array
     must exist already */
  int i=1;
  char* temp;
	
  temp=(char*)malloc(sizeof(char)*32);
  temp=strtok(values,",");
  sscanf(temp,"%g",&(f_array[0]));
  while(((temp=strtok(NULL,","))!=NULL)&&(i<count)){
    sscanf(temp,"%g",&(f_array[i]));
    i++;
  }
  return 0;
}

int make_int_array(char* values, int count, int* i_array){
  /* Read comma-seprated values from string and put them into
     the int array, which has #count entries. The int array
     must exist already */
  int i=1;
  char* temp;
  /* printf("DEBUG %s, line %i begin make_int_array.\n",__FILE__,__LINE__); */
  temp=(char*)malloc(sizeof(char)*32);
  temp=strtok(values,",");
  if(strlen(values)<1){return -4032;}  /* for safety reasons */
  sscanf(temp,"%i",&(i_array[0]));
  while(((temp=strtok(NULL,","))!=NULL)&&(i<count)){
    sscanf(temp,"%i",&(i_array[i]));
    i++;
  }
  return 0;
}

int handle_cmd_line_options(int argc, char* argv[]){
  /* handles all command line arguments. Returns a value > 0 in case the main program should not perform
     a simultion but something else:
     1,2: nothing
     3:   conversion of mat file to element files
  */
  int i=0;
  int result=0;
  while((++i)<argc){ /* Go through command line args */
    if(strcmp(argv[i],"-h")==0){
      print_help_text();
      return 1; /* so the main function knows not to proceed */
    }
    if(strcmp(argv[i],"-l")==0){
      if(display_a_file("license.txt")!=0){
	message_error(0,"cannot open license.txt\nSee: http://www.gnu.org/licenses/gpl.html\n.");
      }
      return 2; /* so the main function knows not to proceed */
    }
    if(strcmp(argv[i],"-c")==0){ /* option for alternative config file name */
      i++;
      if((i<argc)&&(strlen(argv[i])<=1023)){
	strcpy(ConfigFileName,argv[i]);
      } else {
	message_error(-4001,"the -c option requires a config file name (of less than 1024 characters)\n");
	return -4001;
      }
    }
    if(strcmp(argv[i],"-data")==0){ /* option for alternative directory data corteo */
      i++;
      if((i<argc)&&(strlen(argv[i])<=1023)){
	strcpy(DirectoryData,argv[i]);
      } else {
	message_error(-4001,"the -data option requires a directory name (of less than 1024 characters)\n");
	return -4001;
      }
    }
    if(strcmp(argv[i],"-p")==0){ /* option for setting print level */
      i++;
      if(i<argc){
	if(sscanf(argv[i],"%i",&print_level)!=1){
	  message_error(-4002,"cannot read level of verbosity from command line option -p\n");
	  return -4002;
	}
      } else {
	message_error(-4003,"the -p option requires a following integer number!\n");
	return -4003;
      }
    }
    if(strcmp(argv[i],"-n")==0){ /* option for overriding maximum ion number */
      i++;
      if(i<argc){
	if(sscanf(argv[i],"%i",&override_max_ions)!=1){
	  message_error(-4004,"cannot read level maximum ion number from command line option -n\n");
	  return -4004;
	}
      } else {
	message_error(-4005,"the -n option requires a following integer number!\n");
	return -4005;
      }
    }
    if(strcmp(argv[i],"-g")==0){ /* option to start iradina from another prog */
      i++;
      if(i<argc){
	create_status_file=1;
	start_id_string=(char*)malloc(129*sizeof(char));
	strncpy(start_id_string,argv[i],129);
	start_id_string[129]='\0';
      } else {
	message_error(-4007,"the -g option requires a following string!\n");
	return -4007;
      }
    }
    if(strcmp(argv[i],"-E")==0){ /* option to override ion energy */
      i++;
      if(i<argc){
	if(sscanf(argv[i],"%lg",&override_energy)!=1){
	  message_error(-4008,"cannot read override energy from option -E\n");
	  return -4008;
	}
      } else {
	message_error(-4009,"the -E option requires a following float number!\n");
	return -4009;
      }
    }
    if(strcmp(argv[i],"-w")==0){ /* Do not end program before pressing enter.
				    Useful to check the memory usage of the program */
      wait_before_end=1;
    }
    if(strcmp(argv[i],"-m")==0){ /* Do not simulate, just estimate memory usage of the program */
      mem_usage_only=1;
    }
    if(strcmp(argv[i],"-d")==0){ /* print memory usage details */
      mem_usage_details=1;
    }
    if(strcmp(argv[i],"-i")==0){ /* print some info on this version of iradina */
      print_version_info(stdout);
      return 1; /* tell main function to exit */
    }
    if(strcmp(argv[i],"-s")==0){ /* option to print stopping table */
      i++;
      if(i<argc){
	if(sscanf(argv[i],"%i",&stopping_target_index)!=1){
	  message_error(-4035,"cannot read target material for option -s\n");
	  return -4035;
	} else { /* Successfully read: */
	  return 4;
	}
      } else {
	message_error(-4036,"the -s option requires a following integer number!\n");
	return -4036;
      }
    }
    if(strcmp(argv[i],"-conv")==0){ /* option for converting a material based definition to element based */
      i++;
      if((i<argc)&&(strlen(argv[i])<=1023)){
	strcpy(ConversionFileName,argv[i]);
	result=3;
      } else {
	message_error(-4010,"the -conv option requires an output file name (of less than 1024 characters), where the element definition will be stored.\n");
	return -4010;
      }
    }
    if(strcmp(argv[i],"-convsep")==0){ /* creates separate elements for each material */
      conv_create_separate_elements=1;
    }
  }
  return result;
}

int print_help_text(){
  /* prints the help */
  printf("Usage: iradina [OPTIONS]\n");
  printf(" Available options: \n");
  printf(" -h            print this help\n");
  printf(" -l            display license\n");
  printf(" -c FILENAME   specify name of config file. Default: Config.in\n");
  printf(" -data DATADIR specify name of corteo database directory. Default: ./data\n");
  printf(" -p NUMBER     specify how much info to print to console. > 0 means much,\n");
  printf("               < 0 means little\n");
  printf(" -n NUMBER     sets the maximum number of ions to be simulated to NUMBER.\n");
  printf("               This option overrides the setting from the config file. \n");
  printf(" -E NUMBER     sets the energy of the ions.\n");
  printf("               This option overrides the setting from the config file. \n");
  printf(" -w            wait for return key before exiting \n");
  printf(" -m            do not simulate, only estimate memory usage (roughly) \n");
  printf(" -d            print details for memory usage (only useful with -m option) \n");
  printf(" -g ID         generate status file while running \n");
  printf(" -i            print info on this version of iradina \n");
  printf(" -s NUMBER     print electronic stopping table for ion in material of\n");
  printf("               index NUMBER. No simulation is done.\n");

  /*  printf(" -conv FILE    Converts material based input files to element based input\n");
      printf("               file. If the input file is combined, then the output file\n");
      printf("               will also be combined and written to FILE. Otherwise, the\n");
      printf("               elements are stored in FILE and '.e' is appended to the\n");
      printf("               new compositon file name.\n");
      printf(" -convsep      Should only be used with conv. Create separate elements for\n");
      printf("               each material.\n"); */
  return 0;
}

int store_results(char* BaseName,int ion_number){
  /* Store the results of the simulation (arrays with distribution of implanted ions, defects etc.) */
  /* This is the non-dynamic version of the function for material-based output */

  int BaseNameLength;
  char* strTemp;
  char* titleel;
  char* nameel;
  char* strTemp2;
  float maxion,maxvac,vmaxi,vmaxv ; /* CROC */
  double factor;
  int i,j,mat,elem;
  int x,y,z;
	
	
  FILE* fp;
  FILE* fpinfo=NULL; /* for additioanl information */

  BaseNameLength=strlen(BaseName);
  strTemp=(char*)malloc(sizeof(char)*(BaseNameLength+100));
  strTemp2=(char*)malloc(sizeof(char)*(255));
  strncpy(strTemp,BaseName,BaseNameLength+1);
  titleel=(char*)malloc(sizeof(char)*(100));
  nameel=(char*)malloc(sizeof(char)*(100));
  if(store_info_file==1){ /* Create an additional output file with information on iradina and on the simulation */
    strcat(strTemp,".information");
    message(2,"Storing additional information to: %s\n",strTemp);
    fpinfo=fopen(strTemp,"w");
    if(fpinfo==NULL){
      message_error(-4033,"cannot open output file %s\n",strTemp);
      return -4033;
    }
    print_some_simulation_parameters(fpinfo,ion_number);
    print_version_info(fpinfo);
    strTemp[BaseNameLength]='\0';  /* reset strTemporary filename for next storing operation */
  }
  /*  printf("DEBUG %s, l %i\n",__FILE__,__LINE__);fflush(stdout);*/

  /* Arrays to be stored:
     - the array with concentration of implanted ions
     - sum of displacements, replacement and so on in each cell
     - for each element of each material: vacancies and interstitial arrays
     - transmitted ions
     - sputtered atoms in each direction
  */

  sum_up_material_arrays(); /* sum up the individual element-dependent arrays */
  /* determine maximum ion count and maximum vacancy count: */
  maxion=0;maxvac=0;
  for (i = 0; i < cell_count; ++i) {
    if ( TargetImplantedIons[i] > maxion ) {maxion=TargetImplantedIons[i];}
  }
  for (i = 0; i < cell_count; ++i) {
    if ( TargetTotalVacancies[i] > maxvac ) {maxvac=TargetTotalVacancies[i];}
  }
  if(print_level>1){
    if(normalize_output>=1){
      message(2,"Maximum concentration of implanted ions :   %g\n",maxion * unit_conversion_factor);
      message(2,"Maximum concentration of vacancies :   %g\n",maxvac* unit_conversion_factor);
    }else{
      message(2,"Maximum concentration of implanted ions :   %g\n",maxion );
      message(2,"Maximum concentration of vacancies :   %g\n",maxvac);
    }
  }

  /********************************************************************/
  /* First, we store all properties, which are defined for each cell: */
  /********************************************************************/
  vmaxi=maxion;
  vmaxv=maxvac;	
  if(normalize_output>=1){
    vmaxi=maxion*unit_conversion_factor;
    vmaxv=maxvac*unit_conversion_factor;
  }


  if(store_joined_output==0){ /* output to separate files: classical output */
    /* Concentration of implanted ions: */
    strcat(strTemp,".ions.total");
    message(2,"Storing implanted ions to:      %s\n",strTemp);
    if (no_headers_in_files==0) {WriteFileHeader(strTemp, "Implanted ions", "Ions", "ions" );}
    WriteResArrayToFile(strTemp,TargetImplantedIons,cell_count,TargetCompositionFileType,1);

    //		WriteIntArrayToFile(strTemp,TargetImplantedIons,cell_count,TargetCompositionFileType);
    strTemp[BaseNameLength]='\0';  /* reset strTemporary filename for next storing operation */
    if (dpa_output!=0){ dscaled=(float*)malloc(sizeof(float) * cell_count); }
    if(do_not_store_damage==0){
      /* Part of ions that replaced identical target atoms */
      strcat(strTemp,".ions.replacements");
      message(2,"Storing implanted replacing ions to:   %s\n",strTemp);
      if (no_headers_in_files==0) {WriteFileHeader(strTemp, "Implanted replacing ions", "Ions" ,"repl");}
      WriteResArrayToFile(strTemp,TargetReplacingIons,cell_count,TargetCompositionFileType,1);
      strTemp[BaseNameLength]='\0';  /* reset strTemporary filename for next storing operation */
		
      /* Sum of vacancies: */
      strcat(strTemp,".vac.sum");
      message(2,"Storing total vacancies to: %s\n",strTemp);
      message(2, "normalize_output %i\n",normalize_output);
      message(0,"Normalization factor:\t%lg\n",unit_conversion_factor);
      if (no_headers_in_files==0) {WriteFileHeader(strTemp, "Total Vacancies", "Vacancies" ,"vac");}
      WriteResArrayToFile(strTemp,TargetTotalVacancies,cell_count,TargetCompositionFileType,1);
      strTemp[BaseNameLength]='\0';

      /* Sum of displacements: */
      strcat(strTemp,".disp.sum");
      message(2,"Storing total displacements to: %s\n",strTemp);
      if (no_headers_in_files==0) {WriteFileHeader(strTemp, "Total Displacements", "Displacements" ,"disp");}
		
      WriteResArrayToFile(strTemp,TargetTotalDisplacements,cell_count,TargetCompositionFileType,1);
      strTemp[BaseNameLength]='\0';
      /* Sum of recoiled interstitials: */
      strcat(strTemp,".int.sum");
      message(2,"Storing sum of recoiled interstitials:   %s\n",strTemp);
      if (no_headers_in_files==0) {WriteFileHeader(strTemp, "Total Interstitials", "Interstitials", "int" );}
      WriteResArrayToFile(strTemp,TargetTotalInterstitials,cell_count,TargetCompositionFileType,1);
      strTemp[BaseNameLength]='\0';
      /* Sum of recoil replacements: */
      strcat(strTemp,".repl.sum");
      message(2,"Storing sum of recoiled replacements:   %s\n",strTemp);
      if (no_headers_in_files==0) {WriteFileHeader(strTemp, "Total Replacements", "Replacements", "repl" );}
      WriteResArrayToFile(strTemp,TargetTotalReplacements,cell_count,TargetCompositionFileType,1);
      strTemp[BaseNameLength]='\0';
      /* vacancies + ions: (introduced by CROC) */
      strcat(strTemp,".ions_vac");
      message(2,"Storing implanted ions and vacancies:   %s\n",strTemp);

      if (no_headers_in_files==0) {
	fp=fopen(strTemp,"wt");
	fprintf(fp,"#NAME: %s \n", strTemp);
	fprintf(fp,"#TITLE: Vacancies and Implanted ions \n");
	fprintf(fp,"#DATE: %s", asctime(timeinfo) );
	fprintf(fp,"#COLUMN_NAMES: xc | yc | zc | vac | impl \n");
	fprintf(fp,"#COLUMN_TITLES: depth | y width | z width| implanted ions | vacancies \n");
	if(normalize_output>=1){
	  fprintf(fp,"#COLUMN_UNITS: nm | nm | nm | maximplant=%g $cm^{-1}$ | maxvac=%g $cm^{-1}$ \n", vmaxi, vmaxv);
	}
	else {
	  fprintf(fp,"#COLUMN_UNITS: nm | nm | nm | maximplant=%g | maxvac=%g \n", vmaxi, vmaxv);
	}
	fprintf(fp,"#COLUMN_FACTOR: %g | %g | %g | 1.0 | 1.0 \n", cell_size_x, cell_size_y,cell_size_z);
	fprintf(fp," \n");
	fclose(fp);
      }
      Write2ArraysToFile(strTemp,TargetImplantedIons,maxion,TargetTotalVacancies,maxvac,cell_count,TargetCompositionFileType);
      strTemp[BaseNameLength]='\0';
    }
    /* Deposited energy: */
    if(store_energy_deposit==1){
      strcat(strTemp,".energy.phonons");
      message(2,"Storing energy deposited to phonons:  %s\n",strTemp);
      if (no_headers_in_files==0) {WriteEnergyFileHeader(strTemp, "energy deposited to phonons", "Energy" ,"enph");}
      WriteDoubleArrayToFile(strTemp,TargetEnergyPhonons,cell_count,TargetCompositionFileType);
      strTemp[BaseNameLength]='\0';
      strcat(strTemp,".energy.electronic");
      message(2,"Storing energy deposited to electrons:%s\n",strTemp);
      if (no_headers_in_files==0) {WriteEnergyFileHeader(strTemp, "energy deposited to electrons", "Energy" ,"enel");}
      WriteDoubleArrayToFile(strTemp,TargetEnergyElectrons,cell_count,TargetCompositionFileType);
      strTemp[BaseNameLength]='\0';
    }
    /* Sum of atoms leaving the target */
    if(detailed_sputtering==1){
      /* Particles leaving the sample (sputtered or implanted deeper): */
      strcat(strTemp,".leaving_directions.sum");
      message(2," Storing sum of leaving atoms to:       %s\n",strTemp);
      sprintf(strTemp2,"+x\t%i\n-x\t%i\n+y\t%i\n-y\t%i\n+z\t%i\n-z\t%i\n"
	      , TotalSputterCounter[0], TotalSputterCounter[1]
	      , TotalSputterCounter[2], TotalSputterCounter[3]
	      , TotalSputterCounter[4], TotalSputterCounter[5]);
      WriteStringToFile(strTemp,strTemp2);
      strTemp[BaseNameLength]='\0';

      strcat(strTemp,".leaving.sum");
      message(2,"Storing sum of leaving atoms (cells):   %s\n",strTemp);
      WriteIntArrayToFile(strTemp,TargetTotalSputtered,cell_count,TargetCompositionFileType);
      strTemp[BaseNameLength]='\0';

      /* Ions leaving the target */
      strcat(strTemp,".leaving_directions.ions");
      message(2," Storing sum of leaving ions to:        %s\n",strTemp);
      sprintf(strTemp2,"+x\t%i\n-x\t%i\n+y\t%i\n-y\t%i\n+z\t%i\n-z\t%i\n"
	      , leaving_ions[0], leaving_ions[1]
	      , leaving_ions[2], leaving_ions[3]
	      , leaving_ions[4], leaving_ions[5]);
      WriteStringToFile(strTemp,strTemp2);
      strTemp[BaseNameLength]='\0';
    }

    /* Sum of ints and vacs and so on for each element in each material */
    for(i=0;i<NumberOfMaterials;i++){
      if(ListOfMaterials[i].Is_Vacuum==0){ /* do not store stuff for vacuum */
	for(j=0;j<ListOfMaterials[i].ElementCount;j++){ /* Go through elements, store arrays for each */

	  message(2,"Storing for material %i, elem %i:\n",i,j);

	  if(do_not_store_damage==0){
	    sprintf(strTemp+BaseNameLength,".int.z%i.m%.3f.mat%i.elem%i",
		    ListOfMaterials[i].ElementsZ[j],
		    ListOfMaterials[i].ElementsM[j],
		    i,j);
	    message(2," Recoil interstitials to %s\n",strTemp);
	    nameel[0]='\0';
	    titleel[0]='\0';
	    sprintf(titleel, "Interstitials of type Z=%i ",ListOfMaterials[i].ElementsZ[j]);
	    sprintf(nameel, "iZ%i ",ListOfMaterials[i].ElementsZ[j]);
	    if (no_headers_in_files==0) {WriteFileHeader(strTemp, titleel, "Interstitials", nameel);}
	    WriteResArrayToFile(strTemp,ListOfMaterials[i].TargetImplantedRecoilsInt[j],cell_count,TargetCompositionFileType,ListOfMaterials[i].ElementsConc[j]);
	    strTemp[BaseNameLength]='\0';

	    sprintf(strTemp+BaseNameLength,".repl.z%i.m%.3f.mat%i.elem%i",
		    ListOfMaterials[i].ElementsZ[j],
		    ListOfMaterials[i].ElementsM[j],
		    i,j);
	    message(2," Recoil replacements to %s\n",strTemp);
	    sprintf(titleel, "Replacements of type Z=%i ",ListOfMaterials[i].ElementsZ[j]);
	    sprintf(nameel, "rZ%i ",ListOfMaterials[i].ElementsZ[j]);
	    if (no_headers_in_files==0) {WriteFileHeader(strTemp, titleel, "Replacements", nameel);}
	    WriteResArrayToFile(strTemp,ListOfMaterials[i].TargetImplantedRecoilsRepl[j],cell_count,TargetCompositionFileType,ListOfMaterials[i].ElementsConc[j]);
	    strTemp[BaseNameLength]='\0';

	    sprintf(strTemp+BaseNameLength,".vac.z%i.m%.3f.mat%i.elem%i",
		    ListOfMaterials[i].ElementsZ[j],
		    ListOfMaterials[i].ElementsM[j],
		    i,j);
	    message(2," Vacancies to     %s\n",strTemp);
	    sprintf(titleel, "vacancies of type Z=%i ",ListOfMaterials[i].ElementsZ[j]);
	    sprintf(nameel, "vZ%i ",ListOfMaterials[i].ElementsZ[j]);
	    if (no_headers_in_files==0) {WriteFileHeader(strTemp, titleel, "Vacancies", nameel);}
	    WriteResArrayToFile(strTemp,ListOfMaterials[i].TargetElementalVacancies[j],cell_count,TargetCompositionFileType,ListOfMaterials[i].ElementsConc[j]);
	    strTemp[BaseNameLength]='\0';

	    sprintf(strTemp+BaseNameLength,".disp.z%i.m%.3f.mat%i.elem%i",
		    ListOfMaterials[i].ElementsZ[j],
		    ListOfMaterials[i].ElementsM[j],
		    i,j);
	    message(2," Displacements to   %s\n",strTemp);
	    sprintf(titleel, "Displacements of type Z=%i ",ListOfMaterials[i].ElementsZ[j]);
	    sprintf(nameel, "dZ%i ",ListOfMaterials[i].ElementsZ[j]);
	    if (no_headers_in_files==0) {WriteFileHeader(strTemp, titleel, "Displacements", nameel);}
	    WriteResArrayToFile(strTemp,ListOfMaterials[i].TargetElementalDisp[j],cell_count,TargetCompositionFileType,ListOfMaterials[i].ElementsConc[j]);
	    strTemp[BaseNameLength]='\0';
	  }

	  if(detailed_sputtering==1){
	    sprintf(strTemp+BaseNameLength,".leaving.z%i.m%.3f.mat%i.elem%i",
		    ListOfMaterials[i].ElementsZ[j],
		    ListOfMaterials[i].ElementsM[j],
		    i,j);
	    message(2," Leaving from cell  %s\n",strTemp);
	    WriteResArrayToFile(strTemp,ListOfMaterials[i].TargetSputteredAtoms[j],cell_count,TargetCompositionFileType,ListOfMaterials[i].ElementsConc[j]);
	    strTemp[BaseNameLength]='\0';
	  }
	}
      }
    }
  } else { /* output joined file for ions, vacs, ints, energy, etc. into one big tab-separated table */
    //printf("DEBUG %s, l %i\n",__FILE__,__LINE__);fflush(stdout);
    message(2,"Storing results to joined output table...\n");
    sprintf(strTemp+BaseNameLength,".joined_results");
    fp=fopen(strTemp,"wt");

    if(fp==NULL){return -5006;}
    if (no_headers_in_files==1)
      {   /* Write column heads */
	if(TargetCompositionFileType==0){ /* write coordinates */
	  fprintf(fp,"#x\ty\tz\t");
	} else {
	  fprintf(fp,"#");
	}

	fprintf(fp,"ions.total\t");
	if(do_not_store_damage==0){
	  fprintf(fp,"ions.replacements\t");
	  fprintf(fp,"vac.sum\t");
	  fprintf(fp,"disp.sum\t");
	  fprintf(fp,"int.sum\t");
	  fprintf(fp,"repl.sum\t");
	  fprintf(fp,"ions.norm_to_1\t");
	  fprintf(fp,"vac.norm_to_1\t");
	  if(detailed_sputtering==1){
	    fprintf(fp,"leaving.sum\t");
	  }
	  for(mat=0;mat<NumberOfMaterials;mat++){
	    if(ListOfMaterials[mat].Is_Vacuum==0){ /* do not store stuff for vacuum */
	      for(elem=0;elem<ListOfMaterials[mat].ElementCount;elem++){ /* Go through elements, store values for each */
		fprintf(fp,".vac.z%i.m%.3f.mat%i.elem%i\t",ListOfMaterials[mat].ElementsZ[elem],ListOfMaterials[mat].ElementsM[elem],mat,elem);
		fprintf(fp,".disp.z%i.m%.3f.mat%i.elem%i\t",ListOfMaterials[mat].ElementsZ[elem],ListOfMaterials[mat].ElementsM[elem],mat,elem);
		fprintf(fp,".int.z%i.m%.3f.mat%i.elem%i\t",ListOfMaterials[mat].ElementsZ[elem],ListOfMaterials[mat].ElementsM[elem],mat,elem);
		fprintf(fp,".repl.z%i.m%.3f.mat%i.elem%i\t",ListOfMaterials[mat].ElementsZ[elem],ListOfMaterials[mat].ElementsM[elem],mat,elem);
		//fprintf(fp,".int.z%i.m%.3f.mat%i.elem%i\t",ListOfMaterials[mat].ElementsZ[elem],ListOfMaterials[mat].ElementsM[elem],mat,elem);
		if(detailed_sputtering==1){
		  fprintf(fp,".leaving.z%i.m%.3f.mat%i.elem%i\t",ListOfMaterials[mat].ElementsZ[elem],ListOfMaterials[mat].ElementsM[elem],mat,elem);
		}
	      }
	    }
	  }
	}
	/* Deposited energy */
	if(store_energy_deposit==1){
	  fprintf(fp,"energy.phonons\t");
	  fprintf(fp,"energy.electronic\t");
	}
	fprintf(fp,"\n");
	//printf("DEBUG %s, l %i\n",__FILE__,__LINE__);fflush(stdout);
      }else
      {   /* headers in file*/
	strTemp[BaseNameLength]='\0';  /* reset strTemporary filename for next storing operation */
	strcat(strTemp,".joined_results");
	fprintf(fp,"#NAME: %s \n", strTemp);
	fprintf(fp,"#TITLE: Joined output ");
	if((ion_dose==-1.0)||(ion_dose==1)){
	  fprintf(fp," \n");
	}
	else {
	  if(dpa_output==1){
	    fprintf(fp,"dpa output") ;
	  }
	  fprintf(fp," iondose= %g \n",ion_dose) ;
	}
	fprintf(fp,"#DATE: %s", asctime(timeinfo) );
	fprintf(fp,"#COLUMN_NAMES: xc | yc | zc | ions | replions | vac | disp | int | repl | ionN | vacN | ");
	if(detailed_sputtering==1){
	  fprintf(fp," leav |");
	}
	for(mat=0;mat<NumberOfMaterials;mat++){
	  if(ListOfMaterials[mat].Is_Vacuum==0){ /* do not store stuff for vacuum */
	    for(elem=0;elem<ListOfMaterials[mat].ElementCount;elem++){ /* Go through elements, store values for each */
	      fprintf(fp," vZ%i ",ListOfMaterials[mat].ElementsZ[elem]);
	      fprintf(fp," | dZ%i ",ListOfMaterials[mat].ElementsZ[elem]);
	      fprintf(fp," | iZ%i ",ListOfMaterials[mat].ElementsZ[elem]);
	      fprintf(fp," | rZ%i ",ListOfMaterials[mat].ElementsZ[elem]);
	      if(detailed_sputtering==1){
		fprintf(fp," | lZ%i ",ListOfMaterials[mat].ElementsZ[elem]);
	      }
	    }
	  }
	}
	if(store_energy_deposit==1){
	  fprintf(fp," | enph | enel ");
	}
	fprintf(fp,"\n");
		  		    
	fprintf(fp,"#COLUMN_TITLES: depth | y width | z width| implanted ions | replacing ions | vacancies | displaced atoms | interstitials | replacements | normalized implanted ions | normalized vacancies | ");
	if(detailed_sputtering==1){
	  fprintf(fp," sputtered atoms |");
	}

	for(mat=0;mat<NumberOfMaterials;mat++){
	  if(ListOfMaterials[mat].Is_Vacuum==0){ /* do not store stuff for vacuum */
	    for(elem=0;elem<ListOfMaterials[mat].ElementCount;elem++){ /* Go through elements, store values for each */
	      fprintf(fp," vacancies of type Z=%i ",ListOfMaterials[mat].ElementsZ[elem]);
	      fprintf(fp," | displacements of type Z=%i ",ListOfMaterials[mat].ElementsZ[elem]);
	      fprintf(fp," | interstitials of type Z=%i ",ListOfMaterials[mat].ElementsZ[elem]);
	      fprintf(fp," | replacements of type Z=%i ",ListOfMaterials[mat].ElementsZ[elem]);
	      if(detailed_sputtering==1){
		fprintf(fp," | sputtered atoms of type Z=%i ",ListOfMaterials[mat].ElementsZ[elem]);
	      }
	    }
	  }
	}
	if(store_energy_deposit==1){
	  fprintf(fp," |  Energy  to phonons | Energy to electrons ");
	}
	fprintf(fp,"\n");

	fprintf(fp,"#COLUMN_UNITS: nm | nm | nm | ");
		    
	if((ion_dose==-1.0)||(ion_dose==1)){
	  if(normalize_output>=1){
	    fprintf(fp,"$cm^{-1}$ per ion | $cm^{-1}$ per ion | $cm^{-1}$ per ion | $cm^{-1}$ per ion | $cm^{-1}$ per ion | $cm^{-1}$ per ion | maximplant=%g | maxvac=%g |", vmaxi, vmaxv);
	  }
	  else {
	    fprintf(fp," | | | | |  maximplant= %g | maxvac= %g ", vmaxi,vmaxv);
	  }
	}
	else    {
	  if(dpa_output==1){
	    fprintf(fp,"dpa | dpa | dpa | dpa | dpa |   maximplant= %g | maxvac= %g", vmaxi,vmaxv);
	  }
	  else {
	    if(normalize_output>=1){
	      fprintf(fp,"$cm^{-1}$ | $cm^{-1}$ | $cm^{-1}$ | $cm^{-1}$ | $cm^{-1}$ | maximplant=%g | maxvac=%g", vmaxi, vmaxv);	    
	    }
	    else {
	      fprintf(fp," | | | | |  maximplant= %g | maxvac= %g ", vmaxi,vmaxv);
	    }
	  }
	}
	if(detailed_sputtering==1){
	  fprintf(fp,"  |  ");
	}

	for(mat=0;mat<NumberOfMaterials;mat++){
	  if(ListOfMaterials[mat].Is_Vacuum==0){ /* do not store stuff for vacuum */
	    for(elem=0;elem<ListOfMaterials[mat].ElementCount;elem++){ /* Go through elements, store values for each */
	      if((ion_dose==-1.0)||(ion_dose==1)){
		if(normalize_output>=1){
		  fprintf(fp,"$cm^{-1}$ per ion | $cm^{-1}$ per ion | $cm^{-1}$ per ion | $cm^{-1}$ per ion");
		}
		else {
		  fprintf(fp," | | | | |");
		}
	      }
	      else {
		if(dpa_output==1){
		  fprintf(fp,"dpa | dpa | dpa | dpa | ");	    }
		else {
		  if(normalize_output>=1){
		    fprintf(fp,"$cm^{-1}$ | $cm^{-1}$ | $cm^{-1}$ | $cm^{-1}$ | ");	    
		  }
		  else {
		    fprintf(fp," | | | | |  ");
		  }
		}
	      }
	      if(detailed_sputtering==1){
		fprintf(fp,"  |  ");
	      }
	    }
	  }
	}
	if(store_energy_deposit==1){
	  if((ion_dose==-1.0)||(ion_dose==1)){
	    if(normalize_output>=1){
	      fprintf(fp," | eV.$cm^{-1}$ per ion | eV.$cm^{-1}$ per ion \n");
	    }
	    else {
	      fprintf(fp," | eV | eV \n");
	    }
	  }
	  else {
	    if(normalize_output>=1){
	      fprintf(fp,"| eV.$cm^{-1}$ | eV.$cm^{-1}$ \n");
	    }
	    else {
	      fprintf(fp," | eV | eV \n");
	    }
			
	  }
	}

 
	fprintf(fp,"#COLUMN_FACTOR: %g | %g | %g | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 | 1.0 | 1.0", cell_size_x, cell_size_y,cell_size_z);
	for(mat=0;mat<NumberOfMaterials;mat++){
	  if(ListOfMaterials[mat].Is_Vacuum==0){ /* do not store stuff for vacuum */
	    for(elem=0;elem<ListOfMaterials[mat].ElementCount;elem++){ /* Go through elements, store values for each */
	      fprintf(fp,"  | 1.0 | 1.0 | 1.0 | 1.0  ");
	    }
	  }
	}

      }
    if(store_energy_deposit==1){
      fprintf(fp," | 1.0 | 1.0 \n ");
    }
    fprintf(fp,"\n ");
    /* now write data lines: */
    //printf("DEBUG %s, l %i\n",__FILE__,__LINE__);fflush(stdout);
    factor=1.0;
    if(normalize_output>=1){factor = unit_conversion_factor;}
    for(i=0;i<cell_count;i++){ /* loop cells */
      if(TargetCompositionFileType==0){ /* write coordinates */
	GetTargetXYZ(i,&x,&y,&z); /* convert linear index to 3 coords */
	fprintf(fp,"%i\t%i\t%i\t",x,y,z);
      }
      //printf("DEBUG %s, l %i. Cell: %i\n",__FILE__,__LINE__,i);fflush(stdout);
      /* write data values: ions and summed up properties: */
      fprintf(fp,"%lg\t",TargetImplantedIons[i]*factor);          /* Concentration of implanted ions */
      if(do_not_store_damage==0){
	fprintf(fp,"%lg\t",TargetReplacingIons[i]*factor);      /* Part of ions that replaced identical target atoms */
	fprintf(fp,"%lg\t",TargetTotalVacancies[i]*factor);     /* Sum of vacancies */
	fprintf(fp,"%lg\t",TargetTotalDisplacements[i]*factor); /* Sum of displacements */
	fprintf(fp,"%lg\t",TargetTotalInterstitials[i]*factor); /* Sum of recoiled interstitials: */
	fprintf(fp,"%lg\t",TargetTotalReplacements[i]*factor);  /* Sum of recoil replacements */
	fprintf(fp,"%lg\t",TargetImplantedIons[i]/maxion);      /* Normalized ion concentration. Introduced by CROC */
	fprintf(fp,"%lg\t",TargetTotalVacancies[i]/maxvac);     /* Normalized vacancy concentration. Introduced by CROC  */
	if(detailed_sputtering==1){
	  fprintf(fp,"%lg\t",TargetTotalSputtered[i]*factor);     /* Sum of atoms leaving the target (sputtered) */
	}
	//printf("DEBUG %s, l %i. Cell: %i\n",__FILE__,__LINE__,i);fflush(stdout);
	/* Sum of ints and vacs and so on for each element in each material: */
	for(mat=0;mat<NumberOfMaterials;mat++){
	  if(ListOfMaterials[mat].Is_Vacuum==0){ /* do not store stuff for vacuum */
	    for(elem=0;elem<ListOfMaterials[mat].ElementCount;elem++){ /* Go through elements, store values for each */
	      fprintf(fp,"%lg\t",(ListOfMaterials[mat].TargetElementalVacancies[elem])[i]*factor);
	      fprintf(fp,"%lg\t",(ListOfMaterials[mat].TargetElementalDisp[elem])[i]*factor);
	      fprintf(fp,"%lg\t",(ListOfMaterials[mat].TargetImplantedRecoilsInt[elem])[i]*factor);
	      fprintf(fp,"%lg\t",(ListOfMaterials[mat].TargetImplantedRecoilsRepl[elem])[i]*factor);
	      if(detailed_sputtering==1){
		fprintf(fp,"%lg\t",(ListOfMaterials[mat].TargetSputteredAtoms[elem])[i]*factor);
	      }
	    }
	    //printf("DEBUG %s, l %i. Mat: %i\n",__FILE__,__LINE__,mat);fflush(stdout);
	  }
	}
      }
      //printf("DEBUG %s, l %i\n",__FILE__,__LINE__);fflush(stdout);
      /* Deposited energy */
      if(store_energy_deposit==1){
	fprintf(fp,"%lg\t",TargetEnergyPhonons[i]*factor);   /* phononic energy loss */
	fprintf(fp,"%lg\t",TargetEnergyElectrons[i]*factor); /* electronic energy loss */
      }
      fprintf(fp,"\n");
    }
    strTemp[BaseNameLength]='\0';
  } /* end of: store joined output file */
  /*******************************************************************/
  /* Second, we store all things, which are not per-cell properties: */
  /*******************************************************************/
  //printf("DEBUG %s, l %i\n",__FILE__,__LINE__);fflush(stdout);
  message(3,"Storing cell-independent results...\n");
  /* store single ion sputter yields: */
  if(single_ion_sputter_yields==1){ 
    strcat(strTemp,".single_ion_sputter_yields");
    message(2,"Storing single ion sputter yields to:   %s\n",strTemp);
    fp=fopen(strTemp,"w");
    if(fp==NULL){
      message_error(-20,"cannot store histogram.\n");
      return -20;
    }
    for(i=0;i<=MAX_SPUTTERED;i++){
      fprintf(fp,"%i\t%i\n",i,sputter_yield_histogram[i]);
    }
    fclose(fp);
    strTemp[BaseNameLength]='\0';
  }
  /* Sum of atoms leaving the target: */
  if(detailed_sputtering==1){
    /* Particles leaving the sample (sputtered or implanted deeper): */
    strcat(strTemp,".leaving_directions.sum");
    message(2," Storing sum of leaving atoms to:       %s\n",strTemp);
    sprintf(strTemp2,"+x\t%i\n-x\t%i\n+y\t%i\n-y\t%i\n+z\t%i\n-z\t%i\n"
	    , TotalSputterCounter[0], TotalSputterCounter[1]
	    , TotalSputterCounter[2], TotalSputterCounter[3]
	    , TotalSputterCounter[4], TotalSputterCounter[5]);
    WriteStringToFile(strTemp,strTemp2);
    strTemp[BaseNameLength]='\0';
		
    /* Store leaving counters for each element */
    for(mat=0;mat<NumberOfMaterials;mat++){
      if(ListOfMaterials[mat].Is_Vacuum==0){ /* do not store stuff for vacuum */
	for(elem=0;elem<ListOfMaterials[mat].ElementCount;elem++){ /* Go through elements, store arrays for each */
	  sprintf(strTemp+BaseNameLength,".leaving_directions.z%i.m%.3f.mat%i.elem%i",
		  ListOfMaterials[mat].ElementsZ[elem],
		  ListOfMaterials[mat].ElementsM[elem],
		  mat,elem);
	  sprintf(strTemp2,"+x\t%i\n-x\t%i\n+y\t%i\n-y\t%i\n+z\t%i\n-z\t%i\n"
		  , ListOfMaterials[mat].SputterCounter[6*elem+0], ListOfMaterials[mat].SputterCounter[6*elem+1]
		  , ListOfMaterials[mat].SputterCounter[6*elem+2], ListOfMaterials[mat].SputterCounter[6*elem+3]
		  , ListOfMaterials[mat].SputterCounter[6*elem+4], ListOfMaterials[mat].SputterCounter[6*elem+5]);
	  WriteStringToFile(strTemp,strTemp2);
	  strTemp[BaseNameLength]='\0';
	}
      }
    }
  }

  /* Ions leaving the target */
  strcat(strTemp,".leaving_directions.ions");
  message(2," Storing sum of leaving ions to:        %s\n",strTemp);
  sprintf(strTemp2,"+x\t%i\n-x\t%i\n+y\t%i\n-y\t%i\n+z\t%i\n-z\t%i\n"
	  , leaving_ions[0], leaving_ions[1]
	  , leaving_ions[2], leaving_ions[3]
	  , leaving_ions[4], leaving_ions[5]);
  WriteStringToFile(strTemp,strTemp2);
  strTemp[BaseNameLength]='\0';
		
  if(store_transmitted_ions==1){
    strcat(strTemp,".transmitted.ions");
    message(2,"Storing transmitted ions to: %s\n",strTemp);
    StoreTransmissionArray(strTemp,transmit_list,transmission_pointer);
    strTemp[BaseNameLength]='\0';  /* reset strTemporary filename for next storing operation */
  }
  if(store_exiting_recoils==1){
    message(2,"Storing transmitted recoils...\n");
    for(i=0;i<NumberOfMaterials;i++){ /* Go through mats */
      if(ListOfMaterials[i].Is_Vacuum==0){ /* do not store stuff for vacuum */
	for(j=0;j<ListOfMaterials[i].ElementCount;j++){ /* Go through elements, store arrays for each */
	  sprintf(strTemp+BaseNameLength,".leaving_recoils.z%i.m%.3f.mat%i.elem%i",
		  ListOfMaterials[i].ElementsZ[j],
		  ListOfMaterials[i].ElementsM[j],
		  i,j);
	  StoreTransmissionArray(strTemp,ListOfMaterials[i].ElementalLeavingRecoils[j],ListOfMaterials[i].leaving_recoils_pointer[j]);
	  strTemp[BaseNameLength]='\0';  /* reset strTemporary filename for next storing operation */
	}
      }
    }
  }

  free(strTemp);
  if(store_info_file==1){ /* close file with additional info */
    if(fpinfo!=NULL){fclose(fpinfo);}
  }

  return 0;
}


int InitConfiguration(char* ConfigFileName){
  /* Read configuration from file, initialize variables etc. */

  int result;
  float length;
  float random;
  int i;
  long unsigned int lui_temp;
  struct transmitted_ion temp;

  /* Some default values */
  straggling_model=0;
  simulation_type=0;

  /* if the config file is a combined input file (including target, materials, and composition),
     then it must be split up into the separate files first: */
  CheckSplitInputFile(ConfigFileName);

  /* Read general config: */
  result=IniFileReader(ConfigFileDataBlockReader, ConfigFileDataReader, ConfigFileName);
  if(result!=0){
    //printf("Error reading config file %s.\n",ConfigFileName);
    message_error(-1,"Error reading config file %s.\n",ConfigFileName);
    return result;
  }
  message(0,"Configuration read from %s.\n",ConfigFileName);
	
  if ((dpa_output!=0)&&(ion_dose==-1)){
    message(2,"dpa_output contradicts ion dose \n");
    exit(1);
  }
	
  if(store_path_limit_recoils==-1){ /* Assume that no limit for storing recoils cascades has been defined. Set to limit of ion paths */
    store_path_limit_recoils=store_path_limit;
    message(3,"No storing limit for recoil cascades defined. Set to limit for ion paths: %i\n",store_path_limit);
  }
	
  /* Handle dose and normalization method: */

  /* Ion dose == 1.0 should be allowed! */
  /*	if((ion_dose==-1.0)||(ion_dose=1)){ assume user has not specified the dose in the input file */
  if(ion_dose==-1.0){ /* assume user has not specified the dose in the input file */
    if(normalize_output==-1){ /* user has not set the normalization method. */
      ion_dose = 1.0;       /* default dose: 1 ion / cm^2 */
      normalize_output = 2; /* default normalization: type 2 */
    } else { /* user has set normalization but has not set dose */
      ion_dose = 1.0;       /* default dose: 1 ion / cm^2 */
    }
  } else { /* the user has set the dose */
    if(normalize_output==-1){ /* user has not set the normalization method. */
      normalize_output = 1; /* default normalization: type 1, because user has set a dose! We assume the user wants to calculate results for implantation */
    } else { /* user has set normalization and dose */
      /* change nothing, assume user desires the specific behaviour as requested */
    }
  }

	
  if(mem_usage_only==0){ /* Do real stuff, not just estimating memory usage */
    /* Load the corteo scattering matrix */
    result=loadMatrix(DirectoryData);
    if(result!=1){message_error(-4014,"cannot load corteo scattering matrix.\n");return -4014; }
    message(0,"Corteo scattering matrix loaded.\n");

    /* call corteo's list generator */
    computelists();
    mySqrtTableFill();
    message(0,"Lists of random numbers generated.\n");

    /* Normalize ion velocity unit vector */
    length=1.0f/sqrt(ion_vx*ion_vx + ion_vy*ion_vy + ion_vz*ion_vz);
    ion_vx*=length;
    ion_vy*=length;
    ion_vz*=length;

    /* Make sure that everything is correctly initialzed: */
    if(OutputFileBaseName==NULL){
      message(-1,"No output file basename specified. Using: default_out/out.\n");
      OutputFileBaseName=(char*)malloc(sizeof(char)*18);
      strcpy(OutputFileBaseName,"default_out/out");
    }

    /* Init array for storing transmitted ions */
    if(store_transmitted_ions==1){
      transmission_pointer=0;
      transmit_list=malloc(sizeof(temp)*(max_no_ions+1));
      if(transmit_list==NULL){
	message_error(-10,"Not enough memory to store transmitted ions!\n");
	return -10;
      }
    }

    /* Read Chu's straggling data */
    result=load_Chu_straggling_values();
    if(result!=0){message_error(result,"cannot read Chu's straggling values!\n");return result;}
    message(0,"Chu's straggling data read.\n");

    /* Read invserse error function list and randomize it*/
    result=LoadInverseErf();
    if(result!=0){message_error(result,"Error reading invser Erf() list!\n");return result;}
    message(0,"Inverse Erf list read.\n");
    randomizelist(inverse_erf_list, MAXERFLIST);
    message(0,"Inverse Erf list randomized.\n");

  } else { /* Estimate memory usage: */
    lui_temp=(DIME*DIMS*sizeof(float));
    mem_usage+=lui_temp;
    if(mem_usage_details==1){message(-1,"MEMORY Scattering matrix (reduced): %li bytes\n",lui_temp);}
    lui_temp=MAXRANLIST*sizeof(float)*2 + MAXLOGLIST*sizeof(float)*2 + MAXAZILIST*sizeof(float)*2;
    mem_usage+=lui_temp;
    if(mem_usage_details==1){message(-1,"MEMORY Random lists:                %li bytes\n",lui_temp);}
    lui_temp=65536*2; /* sqrt lists */
    mem_usage+=lui_temp;
    if(mem_usage_details==1){message(-1,"MEMORY Sqrt lists:                  %li bytes\n",lui_temp);}
    lui_temp=sizeof(temp)*(max_no_ions+1);
    mem_usage+=lui_temp;
    if(mem_usage_details==1){message(-1,"MEMORY Transmitted ions:            %li bytes\n",lui_temp);}
    lui_temp=400*sizeof(float);
    mem_usage+=lui_temp;
    if(mem_usage_details==1){message(-1,"MEMORY Chu straggling:              %li bytes\n",lui_temp);}
    lui_temp=MAXERFLIST*sizeof(float);
    mem_usage+=lui_temp;
    if(mem_usage_details==1){message(-1,"MEMORY ERF list:                    %li bytes\n",lui_temp);}
  }

  /* Read and init materials or element data: */
  result=InitializeMaterials(MaterialsFileName);
  if(result!=0){message_error(result,"Error reading materials file %s.\n",MaterialsFileName);return result;  }
  message(0,"Materials read from %s.\n",MaterialsFileName);
  for(i=0;i<6;i++){ /* Empty ion leaving counter */
    leaving_ions[i]=0;
  }

  /* Read and init target structure */
  result=InitializeTargetStructure(TargetStructureFileName);
  if(result!=0){message_error(result,"Error reading target structure file %s.\n",TargetStructureFileName);return result;  }
  message(0,"Target structure read from %s.\n",TargetStructureFileName);

#ifdef INCLUDE_SPECIAL_GEOMETRY
  if(special_geometry==1){
    result=InitSpecialGeometry();
    if(result!=0){
      message_error(result," cannot init special geometry.\n");
      return result;
    }
  }
#endif

  /* Init some random values (random starting point for lists of random numbers */
  random=d2f(randomx());
  iazimAngle=(unsigned int)(random*MAXAZILIST);
  iranlist=(unsigned int)(random*MAXRANLIST);
  iranloglist=(unsigned int)(random*MAXLOGLIST);
  erflist_pointer=(unsigned int)(random*MAXLOGLIST);

  /* init array for ion single ion sputter yield histogram */
  if(single_ion_sputter_yields==1){
    sputter_yield_histogram=malloc((MAX_SPUTTERED+1)*sizeof(int));
    if(sputter_yield_histogram==NULL){
      message_error(-4015,"Not enough memory store sputter histogram!\n");
      return -4015;
    }
    for(i=0;i<MAX_SPUTTERED;i++){
      sputter_yield_histogram[i]=0;
    }
  }

  calculate_normalization_factor(max_no_ions);
  message(0,"Normalization factor:\t%lg\n",unit_conversion_factor);

  return 0;
}

int PrepareStoppingTables(){
  /* Read stopping data from file and fill arrays etc. */
  /* material version */
  int i,j,k,l;
  int result;
  char StoppingFileName[1000];

  float fltTemp[DIMD+1];

  /* Go through materials and create stopping tables for all existing elements */
  for(i=0;i<NumberOfMaterials;i++){
    ListOfMaterials[i].StoppingZE=(float**)malloc(sizeof(float*)*MAX_ELEMENT_NO); /* pointers to stopping arrays */
    if(ListOfMaterials[i].StoppingZE==NULL){return -1;}
    /* Go through possible elements which might be slowed down in this materials */
    for(j=0;j<MAX_ELEMENT_NO;j++){ /* Goes through all possible projectiles */
      if(existing_elements[j]==1){ /* ok, element might occur, calculate */
	/* Allocate memory for stopping table */
	ListOfMaterials[i].StoppingZE[j]=(float*)calloc(MAX_STOPPING_ENTRIES,sizeof(float));
	if(ListOfMaterials[i].StoppingZE[j]==NULL){return -1;}

	/* Ok, now we can load the stopping table from the file */
	/* The following code to do this is adapted from the corteo code */
	sprintf(StoppingFileName, "%s/%u.asp", DirectoryData, j); /* Filename where stopping data are tabulated */
	message(2,"Open %s\n", StoppingFileName);

	/* For all elements occuring in the current material, we need to load the stopping data and then apply a rule for stopping in compounds */
	for(k=0;k<ListOfMaterials[i].ElementCount;k++){ /* Go through elements of target material */

	  result=FloatBlockReader(StoppingFileName,(DIMD+1)*(ListOfMaterials[i].ElementsZ[k]-1),DIMD+1,fltTemp);
	  if(result!=0){ /* Could not read float data block from file */
	    return -2;
	  }

	  if(fltTemp[DIMD]!=ListOfMaterials[i].ElementsZ[k]) { /* each record must end with the element number as control data */
	    return -3; 
	  }
	  for(l=0; l<DIMD; l++) { /* Add stopping for current target element to stopping of current target material */
	    /* electronic stopping of index k; factor 10 is conversion from eV/(1E15 at/cm2) to eV/(at/A2), density is in at/A3 */
	    (ListOfMaterials[i].StoppingZE[j])[l]+=fltTemp[l] 
	      * ListOfMaterials[i].ElementsConc[k] 
	      * 10.0f
	      * ListOfMaterials[i].Density*1e-24  /* Must be in at/A^3 CHECK */
	      /* * compoundCorr; */ /* Bragg's rule */
	      ;
	    /* The compound correction needs to be added here !!! */
	    /* following copied from corteo:
	       compound correction according to Zeigler & Manoyan NIMB35(1998)215, Eq.(16) (error in Eq. 14)
	       if(compoundCorr!=1.0f)
	       for(k=0; k<DIMD; k++) {
	       f = d2f( 1.0/(1.0+exp( 1.48*( sqrt(2.*Dval(k)/projectileMass/25e3) -7.0) )) );
	       spp[k]*=f*(compoundCorr-1.0f)+1.0f;
	       }
	    */

	    /* Unit here is eV/nm */
	  } /* End of loop through index stopping values */
	} /* End of loop through elements of target material */
      } else { /* if element does not occur, create NULL pointer */
	ListOfMaterials[i].StoppingZE[j]=NULL;
      }
    } /* End of loop through possible projectile elements */
  } /* End of loop through target materials */
  return 0;
}
int PrepareStragglingTables(int model){
  /* Create straggling tables etc. */
  /* material version! */
  /*  model:
      0: no straggling
      1: Bohr straggling
      2: Chu correction        PRA  13 (1976) 2057
      3: Chu + Yang correction NIMB 61 (1991) 149 */
  /* The function load_Chu_straggling_values() must have been called before */

  int i,Z,k,l;        /* projectile's Z */
  unsigned long ii;

  double straggling;
  double stragg_element;
  double stopping;
  double energy;
  double MEV_energy_amu;
  double chargestate2; /* the effective charge state of current projectile */
  double mass;         /* mass of most abundant isotope of projectile Z */
  int    target_Z;
  double OmegaBohr2;
  double Chu_factor;  /* Chu's correction factor for the Bohr straggling */
  double Yang,epsilon,Gamma;
  double C1,C2,C3,C4,B1,B2,B3,B4;

  message(2,"Straggling model %i\n",model);

  /* Go through materials and create straggling tables for all existing elements */
  for(i=0;i<NumberOfMaterials;i++){

    ListOfMaterials[i].StragglingZE=(float**)malloc(sizeof(float*)*MAX_ELEMENT_NO); /* pointers to straggling arrays */
    if(ListOfMaterials[i].StragglingZE==NULL){return -4016;} /* Cannot allocate memory */

    for(Z=0;Z<MAX_ELEMENT_NO;Z++){ /* Loop through all possible projectiles */
      if(existing_elements[Z]==1){ /* ok, element might occur as projectile, calculate */

	/* Allocate memory for straggling table of projectile j in material i */
	ListOfMaterials[i].StragglingZE[Z]=(float*)calloc(MAX_STOPPING_ENTRIES,sizeof(float));
	if(ListOfMaterials[i].StragglingZE[Z]==NULL){return -4017;}

	mass=MostAbundantIsotope[Z];

	/* Calculation of straggling (similar to corteo code): */
	for(k=0;k<DIMD;k++){ /* go through table that has to be filled */
	  straggling=0;

	  for(l=0;l<ListOfMaterials[i].ElementCount;l++){ /* All elements of current target material */

	    target_Z=ListOfMaterials[i].ElementsZ[l];
	    stopping=(ListOfMaterials[i].StoppingZE[Z])[k];
	    energy=Dval(k); /* energy corresponding to index k */

	    /* the effective charge state is obtained from comparing stopping of the ion and the hydrogen:
	       chargestateqaured = stopping(H)/stopping(ion) Z_ion^2 for stopping at same speed */
	    ii = Dindex(d2f(energy/mass)); /* index of the velocity which is proton energy of same velocity */
	    chargestate2 = stopping/( ((ListOfMaterials[i].StoppingZE[1])[ii]) * Z * Z);

	    /* Start by calculating squared Bohr straggling (all other models need this anyway) */
	    OmegaBohr2=4.0 * PI * Z * Z * target_Z *  E2 * E2 * ListOfMaterials[i].Density*1e-24  /* Must be in at/A^3 CHECK */;

	    /* Calculate the Chu correction factor. The formula needs energy[MeV]/mass: */
	    MEV_energy_amu = energy*1e-6/mass;
	    if(target_Z>1){
	      Chu_factor=1.0 / ( 1.0
				 + chu_values[target_Z][0] * pow(MEV_energy_amu, chu_values[target_Z][1])
				 + chu_values[target_Z][2] * pow(MEV_energy_amu, chu_values[target_Z][3])   );
	    } else {
	      /* Chu values undefined for target_Z==1,  because the chu table has no data on hydrogen --> use Bohr.*/
	      Chu_factor=1.0;
	    }


	    /* To calculate Yang's extra straggling contribution caused by charge state fluctuations,
	       we need his Gamma and his epsilon (eq.6-8 from the paper): */
	    /* For hydrogen we need the B and for other projectile the C constants: */
	    if(Z==1){
	      B1 = 0.1955;     B2 = 0.6941;     B3 = 2.522;  B4 = 1.040;
	      Gamma = B3 * (1.0- exp(-B4 * MEV_energy_amu));
	      Yang  = B1 * Gamma / ( pow((MEV_energy_amu-B2),2.0) + Gamma*Gamma  );
	    } else {
	      C1 = 1.273e-2;   C2 = 3.458e-2;   C3 = 0.3931;   C4 = 3.812;    /* solid targets */
	      epsilon = MEV_energy_amu *  pow(Z,-1.5)  * pow(target_Z,-0.5);  /* solid targets */
	      Gamma = C3 * (1.0- exp(-C4 * epsilon));
	      Yang  = (  pow(Z,1.333333333333)/pow(target_Z,0.33333333333) ) *  C1 * Gamma / ( pow((epsilon-C2),2.0) + Gamma*Gamma  );
	    }

	    /* Now we have all ingredients for any of the straggling models. We could have saved some
	       calculations by checking the model first, but well... we'll probably use Yang's model in most cases */
	    switch(model) {
	    case 0: /* no straggling */
	      stragg_element = 0.;
	      break;
	    case 1: /* Bohr */
	      stragg_element = OmegaBohr2;
	      break;
	    case 2: /* Chu */
	      stragg_element = OmegaBohr2*Chu_factor;
	      break;
	    case 3: /* Chu + Yang correction */
	      stragg_element = OmegaBohr2*(chargestate2*Chu_factor+Yang);
	      break;
	    default:
	      stragg_element=0;
	    }

	    /* Now we know the straggling for each target element in the material we can add them up using Bragg's rule of additivity */
	    straggling += stragg_element * ListOfMaterials[i].ElementsConc[l];

	  } /* end of loop through elements in current target material, l */

	  /* Store the straggling in its table: */
	  (ListOfMaterials[i].StragglingZE[Z])[k] = sqrtdf(straggling)*sqrtdf(2.0);

	} /* end of loop through entries in straggling table, k */
      } /* end the check if element might occur as projectile */
    } /* end of possible projectiles loop */
  } /* end of target material loop */

  return 0;
}

int load_Chu_straggling_values(){
  /* This function is adapted from corteo */
  /* returns 0 on success */
  /* Loads the Chu straggling data (from Q.Yang et al., NIMB vol. 61, page 149, (1991)) */
  unsigned int k, l, Z;
  char temp[1000];
  FILE * fp;
  char dataFile[1000];
  sprintf(dataFile, "%s/chu.dat", DirectoryData); /* dir + chu.dat */
  message(2,"Open %s\n", dataFile);
  fp = fopen(dataFile, "r");
  if(fp==NULL) {
    return -1;
  }
  ignoreline(fp); /* skip first line */
  for(k=0; k<92; k++) {
    fscanf(fp,"%u", &Z);
    for(l=0; l<4; l++) {
      fscanf(fp,"%s", temp);
      chu_values[Z][l] = a2f(temp);
    }
    if(Z!=k+2){ /* Check consistency */
      return -2;
    }
  }
  fclose(fp);
  return 0;
}

int sum_up_material_arrays(){
  /* Interstitials and so on are stored for each element from each material
     separately, but may also be interesting in sum. So this function does
     all the summing up. */
  int k;
  int i,j;

  /* Reset sum arrays: */
  fill_zero(TargetTotalVacancies,cell_count);
  fill_zero(TargetTotalDisplacements,cell_count);
  fill_zero(TargetTotalInterstitials,cell_count);
  fill_zero(TargetTotalReplacements,cell_count);

  if(detailed_sputtering==1){
    /* Arrays of leaving atoms */
    for(k=0;k<6;k++){TotalSputterCounter[k]=0;} /* Reset sum */

    fill_zero(TargetTotalSputtered,cell_count);
  }

  for(i=0;i<NumberOfMaterials;i++){ /* Loop through materials */
    if(ListOfMaterials[i].Is_Vacuum==0){ /* Only for "real"  materials */
      for(j=0;j<ListOfMaterials[i].ElementCount;j++){ /* Loop through elements */
	add_int_array(TargetTotalVacancies,     (ListOfMaterials[i].TargetElementalVacancies)[j]  ,cell_count);
	add_int_array(TargetTotalReplacements,  (ListOfMaterials[i].TargetImplantedRecoilsRepl)[j],cell_count);
	add_int_array(TargetTotalDisplacements, (ListOfMaterials[i].TargetElementalDisp)[j]       ,cell_count);
	add_int_array(TargetTotalInterstitials, (ListOfMaterials[i].TargetImplantedRecoilsInt)[j] ,cell_count);
	if(detailed_sputtering==1){
	  add_int_array(TargetTotalSputtered,     (ListOfMaterials[i].TargetSputteredAtoms)[j]      ,cell_count);

	  for(k=0;k<6;k++){ /* Loop through all 6 directions */
	    TotalSputterCounter[k]+=ListOfMaterials[i].SputterCounter[(6*j)+k];
	  }
	}
      }
    }
  }
  return 0;
}

int fill_zero(int* array, int count){
  /* Fill an arrays with zeros */
  int i;
  for(i=0;i<count;i++){
    array[i]=0;
  }
  return 0;
}


void add_int_array(int* dest, int* source, int count){
  /* adds array source to array dest, both arrays must have count entries*/
  int i;
  for(i=0;i<count;i++){
    dest[i]+=source[i];
  }
}

int count_existing_elements(int* elementarray){
  /* returns the number of ones in the provided array */
  int result=0;
  int i;
  for(i=0;i<MAX_ELEMENT_NO;i++){
    result+=elementarray[i];
  }
  return result;
}

int calculate_normalization_factor(int num_of_ions){
  /* for converting units to 1/cm^3 per 1/cm^2 */
  /* Note: calculation of the normalization factor depends on how dose is defined */
  /* See manual for details */
  message(2,"ion_dose %g\n",ion_dose);

  switch(normalize_output){
  case 1: /* Borschel version with ion_vx: Assume user wants to get results for ion dose measured perpendicular to ion beam */
    unit_conversion_factor = ion_dose * (  (double)(1.0/(cell_size_x*cell_size_y*cell_size_z*1e-21))  ) /
      (((double)(num_of_ions))  /  (ion_vx*target_size_y*target_size_z*1e-14) );
    break;
  case 2: /* CROC version without ion_vx: Assume user wants to get results per one ion, or per desired dose (perpendicular to sample surface). */
    unit_conversion_factor = ion_dose * (  (double)(1.0/(cell_size_x*cell_size_y*cell_size_z*1e-21))  ) /
      (((double)(num_of_ions))  /  (target_size_y*target_size_z*1e-14) );
    break;
  default:  /* for normalize_output==0, we do not multiply with the dose. This will result in pure unscaled simulation results */
    unit_conversion_factor=1.0;
  }
  return 0;
}

int write_status_file(char* status_text, int ion_number){
  /* create a file that holds status information on iradina. Can be used to monitor iradinas status from another program */
  FILE* fpointer;
  fpointer=fopen("ir_state.dat","w");
  if(fpointer==NULL){
    return -1;
  } else {
    fprintf(fpointer,"iradina %i.%i.%i\n",VERSION,SUBVERSION,SUBSUBVERSION);
    fprintf(fpointer,"%s\n",start_id_string);
    fprintf(fpointer,"%s\n",status_text);
    fprintf(fpointer,"%i\n",ion_number);
    fclose(fpointer);
    return 0;
  }
}

double MAGIC(double B, double epsilon){
  /* B: reduced impact par
     epsilon: reduced center of mass energy
     returns cos(theta/2) of the scattering event */

  double cost2;  /* cos(theta/2)*/
  double RoC,Delta,R,RR,A,G,alpha,beta,gamma,V,V1,FR,FR1,Q;
  double SQE;
  double C[6];

  C[1]=0.99229;  /* TRIM 85:  */
  C[2]=0.011615;
  C[3]=0.0071222;
  C[4]=14.813;
  C[5]=9.3066;

  /* Initial guess for R: */
  R=B;
  RR=-2.7*log(epsilon*B);
  if(RR>=B){
    /*   if(RR<B) calc potential; */
    RR=-2.7*log(epsilon*RR);
    if(RR>=B){
      R=RR;
    }
  }
  /* TRIM85: 330 */
  do{
    /* Calculate potential and its derivative */
    V=ZBL_and_deri(R,&V1);
    FR  = B * B / R + V*R/epsilon - R;
    FR1 = - B * B / (R * R) + (V+V1*R)/epsilon - 1.0;
    Q   = FR/FR1;
    R   = R-Q;
  } while(fabs(Q/R)>0.001);

  RoC = -2.0 * (epsilon-V)/V1;
  SQE = sqrt(epsilon);

  alpha = 1+ C[1]/SQE;
  beta  = (C[2]+SQE) / (C[3]+SQE);           /* TRIM85: CC */
  gamma = (C[4]+epsilon)/(C[5]+epsilon);
  A     = 2*alpha*epsilon*pow(B,beta);
  G     = gamma / ( sqrt((1.0+A*A))-A  );    /* TRIM85: 1/FF */
  Delta = A * (R-B)/(1+G);
	
  cost2=(B+RoC+Delta)/(R+RoC);
  return cost2;
}

double ZBL_and_deri(double R, double* Vprime){
  /* return ZBL potential, and via the pointer Vprime its derivative */
  /* Values are taken from ZBL85 */

  double EX1,EX2,EX3,EX4,V;
  /* corteo:    return 0.1818*exp(-3.*x)+0.5099*exp(-0.9423*x)+0.2802*exp(-0.4028*x)+0.02817*exp(-0.2016*x); */
  /* EX1=0.1818   * exp( -3.0  * R);
     EX2=0.5099   * exp( -0.9423 * R);
     EX3=0.2802   * exp( -0.4028  * R);
     EX4=0.02817  * exp( -0.2016  * R);
     V=(EX1+EX2+EX3+EX4)/R;
     *Vprime = -(V+3.0*EX1+0.9423*EX2 + 0.4028*EX3 + 0.2016*EX4)/R;
     return V;*/

  /* TRIM85: */
  EX1=0.18175  * exp( -3.1998  * R);
  /*  if(R>=7){EX1=0.0;}*/ /* According to TRIM95 */
  EX2=0.50986  * exp( -0.94229 * R);
  EX3=0.28022  * exp( -0.4029  * R);
  EX4=0.028171 * exp( -0.20162 * R);
  V=(EX1+EX2+EX3+EX4)/R;
  *Vprime = -(V+3.1998*EX1+0.94229*EX2 + 0.4029*EX3 + 0.20162*EX4)/R;
  return V;
}


void CalculateRelativeTargetAtomPosition(float vx,float vy, float vz,float *px, float *py, float *pz, unsigned int iazimAngle){
  /* This calculates the direction in which the target nucleus is found.
     v is the projectile velocity vector, the IP vector is returned in p components */
  /* This routine works similar to the rotation (DIRCOS) routine. It simply assumes a
     fixed scattering angle of 90 degrees and reverses the resulting vector. Adding the 
     result multiplied with impact_par to the current projectile position leads to
     the target nucleus position. */

  float k, kinv;
  float k2 = 1.0f-vz*vz;

  /* random azimutal rotation angle components */
  float cosomega = cosAzimAngle[iazimAngle];
  float sinomega = sinAzimAngle[iazimAngle];
  /* In the rotation routine, we would have to increase the azim-counter here, but
     since we want to have the same angle for target position and deflection, we must
     not change the index in this function! */
  if(k2<0.000001) {  /* extremely rare case */
    *pz = 0;
    *py = cosomega;
    *px = sinomega;
    return;
  } 

  /* usual case */
#ifndef SAFE_SQR_ROTATION
  kinv = myInvSqrt(k2);  /* 1/sqrt() (approximate) */
  k = k2*kinv;           /* so we get k and 1/k by a table lookup and a multiplication */
#else
  /* these two lines can be replaced by the following two lines but... */
  k  = sqrtdf(k2);   /*  ...using a sqrt() here makes the program 25% slower! */
  kinv = 1.0f/k; 
#endif

  *px = -kinv*(vx*vz*cosomega+vy*sinomega);
  *py = -kinv*(vy*vz*cosomega-vx*sinomega);
  *pz =  k*cosomega;

#ifdef SAFE_ROTATION /* makes iradina slower, but safer */
  if(*px>1){*px=1};
  if(*px<-1){*px=-1};
  if(*py>1){*py=1};
  if(*py<-1){*py=-1};
  if(*pz>1){*pz=1};
  if(*pz<-1){*pz=-1};
#endif
}

float signf(float f){
  /* return signum(f) */
  if(f<0.0){
    return -1.0f;
  }else{
    return +1.0f;
  }
}
double signd(double d){
  /* return signum(d) */
  if(d<0.0){
    return -1.0;
  }else{
    return +1.0;
  }
}


int MaterialToElementConverter(char* OutputFile){
  /* reads in the "standard" config file for material-based target definition and creates files for element based target definition.
     This function only works in the non-dynamic compilation, because this way it is easier to write, since the standard read-procedures
     can be used. */

  int i,j,k;
  int x,y,z;                 /* integer cell indices in each direction */
  struct material* cell_mat; /* pointer to material of current cell */
  FILE* elem_fp;             /* points to element output file */
  FILE* comp_fp;             /* points to new composition file */
  char strTemp[MAX_FILENAME_LENGTH]; /* for temporary filenames */
  int result;

  float meanDisp; /* mean energies are calculated and used for the ion... its simple and may not be correct, */
  float meanLatt; /* but we need some values. Can be changed later by the user */
  float meanSurf;
  float meanMass;
  int ElemCount,ElemCount2;

  float fltTemp;          /* for temporary floats */
  float* compVector;      /* for writing the composition file. Has as many entries as different elements occur */
  int* MatElemVectorP;    /* for each material and element, we will store here the index of the composition
			     vector, which corresponds to the Z of the element. We need this for constructing
			     the composition file. It is not part of the material structure, because it is not
			     used for normal operation of iradina */
  char strTemp2[1024];

  meanDisp=.0f;
  meanLatt=.0f;
  meanSurf=.0f;
  meanMass=.0f;
  ElemCount=0;
  ElemCount2=0;

  message(-1,"\nAttempting to convert material based target definition to element based target composition... \n\n");

  MatElemVectorP=(int*)malloc(MAX_NO_MATERIALS*MAX_EL_PER_MAT*sizeof(int));
  if(MatElemVectorP==NULL){
    message_error(-501,"insufficient memory!\n");
    return -501;
  }

  if(single_input_file==1){ /* ok, create also a single output file. So we need a temporary element file to write to, and a temporary composition file */
    strcpy(ElementsFileName,"temp_elementfile.iradina");
    strcpy(TargetCompositionFileName,"temp_compfile.iradina");
    strcpy(ConversionFileName,OutputFile);
    //    printf("DEBUG %s, %i: cfn: %s\n",__FILE__,__LINE__,ConversionFileName);
  } else { /* use given output file name to put element definition to */
    strcpy(ElementsFileName,OutputFile);
    sprintf(strTemp,"%s.e",TargetCompositionFileName); /* and append .e to new composition file */
    strcpy(TargetCompositionFileName,strTemp);
    //    printf("DEBUG %s, %i: cfn: %s\n",__FILE__,__LINE__,ConversionFileName);
  }
  elem_fp=fopen(ElementsFileName,"w");
  if(elem_fp==NULL){message_error(-4030,"Cannot open file %s for writing.\n",ElementsFileName);return -4030;}

  /* Create element file */
  if(conv_create_separate_elements==1){ /* create separate elements */
    message(-1,"Parsing elements (create separate elements for each material) ...\n");
    fprintf(elem_fp,"ElementCount=%i\n\n",number_of_all_target_elements);
    for(i=0;i<NumberOfMaterials;i++){
      for(j=0;j<ListOfMaterials[i].ElementCount;j++){
	message(1,"Element %s found in material %s. Storing data.\n",AtomicNames[ListOfMaterials[i].ElementsZ[j]],ListOfMaterials[i].Name);
	fprintf(elem_fp,"[%s_from_%s]\n",AtomicNames[ListOfMaterials[i].ElementsZ[j]],ListOfMaterials[i].Name);
	fprintf(elem_fp,"Z=%i\n",ListOfMaterials[i].ElementsZ[j]);
	fprintf(elem_fp,"M=%g\n",ListOfMaterials[i].ElementsM[j]);
	fprintf(elem_fp,"DispEnergy=%g\n",ListOfMaterials[i].ElementsDispEnergy[j]);
	fprintf(elem_fp,"LattEnergy=%g\n",ListOfMaterials[i].ElementsLattEnergy[j]);
	fprintf(elem_fp,"SurfEnergy=%g\n\n",ListOfMaterials[i].ElementsSurfEnergy[j]);
	meanDisp += ListOfMaterials[i].ElementsDispEnergy[j]; /* add stuff up to calc mean values */
	meanLatt += ListOfMaterials[i].ElementsLattEnergy[j];
	meanSurf += ListOfMaterials[i].ElementsSurfEnergy[j];
	MatElemVectorP[i*MAX_EL_PER_MAT+j] = ElemCount+1; /* note, which element number this will get later */
	message(3,"New element number %i assigned to mat. %i, elem. %i\n",ElemCount+1,i,j);
	ElemCount++;
      }
    }
    /* Calculate mean values */
    meanDisp /= (float)ElemCount;
    meanLatt /= (float)ElemCount;
    meanSurf /= (float)ElemCount;
    message(-1,"Finished parsing elements.\n");

  } else {  /* Each element to appear only once */
    message(-1,"Parsing elements (list each elements not more than once) ...\n");
    /* first: count element to put into file: */
    for(i=1;i<MAX_ELEMENT_NO;i++){
      if(existing_elements[i]==1){ /* elements exists! */
	if(  ((i==1)&&(hydrogen_in_target==1))  || 
	     ((i>1)&&(i!=ionZ))    ||
	     ((i==ionZ)&&(ionZ_in_target==1))   ){ /* ok, element really exists in target, not just additionally added ion or hydrogen */
	  ElemCount++;
	}
      }
    }
    fprintf(elem_fp,"ElementCount=%i\n\n",ElemCount);
    ElemCount=0; /* restart counting !*/
    /* then work on elements */
    for(i=1;i<MAX_ELEMENT_NO;i++){
      if(existing_elements[i]==1){ /* elements exists! */
	if(  ((i==1)&&(hydrogen_in_target==1))  || 
	     ((i>1)&&(i!=ionZ))    ||
	     ((i==ionZ)&&(ionZ_in_target==1))   ){ /* ok, element really exists in target, not just additionally added ion or hydrogen */
	  message(1,"Element %s found. Storing data.\n",AtomicNames[i]);
	  /* Now search in which materials the element appears and obtain mean values for mass, energies... : */
	  meanDisp=0;meanLatt=0;meanSurf=0;meanMass=0;ElemCount2=0;
	  for(k=0;k<NumberOfMaterials;k++){
	    for(j=0;j<ListOfMaterials[k].ElementCount;j++){
	      if(ListOfMaterials[k].ElementsZ[j]==i){ /* element found in material */
		MatElemVectorP[k*MAX_EL_PER_MAT+j] = ElemCount+1; /* note, which element number this will get later */
		message(3,"New element number %i assigned to mat. %i, elem. %i. Absolute element: %i\n",ElemCount+1,k,j,i);
		meanDisp += ListOfMaterials[k].ElementsDispEnergy[j]; /* add stuff up to calc mean values */
		meanLatt += ListOfMaterials[k].ElementsLattEnergy[j];
		meanSurf += ListOfMaterials[k].ElementsSurfEnergy[j];
		meanMass += ListOfMaterials[k].ElementsM[j];
		ElemCount2++;
	      }
	    }
	  }
	  meanDisp /= (float)ElemCount2;
	  meanLatt /= (float)ElemCount2;
	  meanSurf /= (float)ElemCount2;
	  meanMass /= (float)ElemCount2;
	  /* Ok, write element info: */
	  fprintf(elem_fp,"[%s]\n",AtomicNames[i]);
	  fprintf(elem_fp,"Z=%i\n",i);
	  fprintf(elem_fp,"M=%g\n",meanMass);
	  fprintf(elem_fp,"DispEnergy=%g\n",meanDisp);
	  fprintf(elem_fp,"LattEnergy=%g\n",meanLatt);
	  fprintf(elem_fp,"SurfEnergy=%g\n\n",meanSurf);
	  ElemCount++;
	} else { /* element is in list, but does not really exist in target, ignore */
	}
      }
    }
    message(-1,"Finished parsing elements.\n");
    /* For now, the ion's displacement, lattice and surface energy are simply determined by the last element that appeard. 
       The values should anyway better be set by the user later! */
  }

  fprintf(elem_fp,"[ion]\n");
  fprintf(elem_fp,"DispEnergy=%g\n",meanDisp);
  fprintf(elem_fp,"LattEnergy=%g\n",meanLatt);
  fprintf(elem_fp,"SurfEnergy=%g\n",meanSurf);

  fclose(elem_fp);

  message(-1,"New element file has been created.\n");
  /* ok, element file has been written, new create composition file */

  compVector=(float*)malloc((ElemCount+1)*sizeof(float));
  if(compVector==NULL){
    message_error(-502,"insufficient memory!\n");
    return -502;
  }

  comp_fp=fopen(TargetCompositionFileName,"w");
  if(comp_fp==NULL){message_error(-4031,"annot open file %s for writing.\n",TargetCompositionFileName);return -4031;}

  message(-1,"Creating new composition file %s ...\n",TargetCompositionFileName);
  for(i=0;i<cell_count;i++){ /* Cycle through all cells and write entries to file */
    GetTargetXYZ(i, &x, &y, &z);                          /* Get coords of cell */
    cell_mat = &(ListOfMaterials[TargetComposition[i]]);  /* Get material of cell */
    //    printf("DEBUG %s, l: %i. Cell: %i, x:%i,y:%i,z:%i mat:%i\n",__FILE__,__LINE__,i,x,y,z,TargetComposition[i]);
    fprintf(comp_fp,"%i\t%i\t%i\t",x,y,z);                /* print coords to file */
    fltTemp = (cell_mat->Density);                        /* Get density */
    //    printf("DEBUG: density:%g\n",fltTemp);
    if(UseDensityMult==1){fltTemp*=TargetDensityMult[i];} /* multiply density, if applicable */
    fprintf(comp_fp,"%g",fltTemp);                      /* print density to file */
    /* Now construct composition vector for current cell: */
    for(j=0;j<=ElemCount;j++){compVector[j]=0;}           /* init with zeros */
    for(j=0;j<cell_mat->ElementCount;j++){                /* cycle through elements of this material and get the concentration */
      compVector[MatElemVectorP[TargetComposition[i]*MAX_EL_PER_MAT+j]] = cell_mat->ElementsConc[j];
      //      printf("elem: %i,conc: %g, CVI: %i\n",j,cell_mat->ElementsConc[j],MatElemVectorP[TargetComposition[i]*MAX_EL_PER_MAT+j]);
    }
    /* Now, write the composition vector into the composition file */
    for(j=0;j<=ElemCount;j++){
      fprintf(comp_fp,"\t%g",compVector[j]);
    }
    fprintf(comp_fp,"\n");
  }
  fclose(comp_fp);
  message(-1,"New composition file %s has been created.\n",TargetCompositionFileName);

  if(single_input_file==1){ /* ok, create a single output file */
    message(-1,"Combining input files to create single project file %s... \n",ConversionFileName);
    sprintf(strTemp2,"ElementsFileName=%s\n\n#<<<BEGIN STRUCTUREFILE",ElementsFileName);
    result=CombineFiles(9,
			ConversionFileName,
			"#<<<BEGIN CONFIGFILE",
			"temp_configfile.iradina",
			strTemp2,
			"temp_structfile.iradina",
			"#<<<BEGIN ELEMFILE",
			ElementsFileName,
			"#<<<BEGIN COMPFILE",
			TargetCompositionFileName);
    if(result!=0){
      message_error(result,"Cannot create combined output file.\n");
      return result;
    }
    message(-1,"Combined input files %s has been created.\n",ConversionFileName);
  }

  message(-1,"\nMaterial based input file successfully converted to element based input file. \n\n");

  return 0;
}

void get_float_one_bit_smaller(float* fltInput,float* fltOutput){
  /* returns the largest float that is smaller than the fltInput */
  /* the following code presumes a IEEE754-conform bit-representation of floats */
  /* it further presumes that int and float have exactly the same bit-length!  */
  int temp; temp=(*((int*)(fltInput)))-1;
  (*fltOutput)=*((float*)(&(temp)));
}

int print_version_info(FILE* fp){
  /* print some machine-readable info on this version of iradina to the stream pointed to by fp */
  char sTemp[254];  /* temporary string */
  int pos=0; /*position in temp string */
  fprintf(fp,"[iradina]\n");
  fprintf(fp,"version=%i.%i.%i\n",VERSION,SUBVERSION,SUBSUBVERSION);
  fprintf(fp,"release=%s\n",RELEASESTRING);
  fprintf(fp,"comment=%s\n",VERSIONCOMMENT);
  fprintf(fp,"versiondate=%s\n",VERSIONDATE);
  fprintf(fp,"compiledate=%s\n",COMPILEDATE);
  fprintf(fp,"compiletime=%s\n",COMPILETIME);
#ifdef INCLUDE_SPECIAL_GEOMETRY
  fprintf(fp,"iradina_type=special_geometry\n");
  fprintf(fp,"special_geometry=%s\n",SPECIAL_GEOMETRY_NAME);
#else
  fprintf(fp,"iradina_type=normal\n");
#endif
  fprintf(fp,"pointersize=%li\n",(long int)sizeof(int*)*8);
#ifdef SAFE_SQR_ROTATION
  strcpy(sTemp+pos,"SAFE_SQR_ROTATION,"); pos=pos+18;
#endif
#ifdef CHECK_NAN_VECTORS
  strcpy(sTemp+pos,"CHECK_NAN_VECTORS,"); pos=pos+18;
#endif
#ifdef RENORM_VELOCITIES
  strcpy(sTemp+pos,"RENORM_VELOCITIES,"); pos=pos+18;
#endif
#ifdef INDEX_BOUND_CHECKING
  strcpy(sTemp+pos,"INDEX_BOUND_CHECKING,"); pos=pos+21;
#endif

  INDEX_BOUND_CHECKING
    sTemp[pos]='\0';
  fprintf(fp,"compiled_options=%s\n",sTemp);
#ifdef PROJ_HANGUP_SAFETY
  fprintf(fp,"proj_hangup_safety=%i\n",PROJ_HANGUP_SAFETY);
#endif
  fprintf(fp,"\n[Lookup_tables]\n");

  /* matrix index calculation parameters */
  /*  fprintf(fp,"MINE=%g\n",MINE);
      fprintf(fp,"MAXE=%g\n",MAXE);
      fprintf(fp,"DIME=%i\n",DIME);
      fprintf(fp,"BIASE=%i\n",BIASE);
      fprintf(fp,"SHIFTE=%i\n",SHIFTE);*/
  fprintf(fp,"energy_mantissa_bits=%i\n",23-SHIFTE);

  /*  fprintf(fp,"MINS=%g\n",MINS);
      fprintf(fp,"MAXS=%g\n",MAXS);
      fprintf(fp,"DIMS=%i\n",DIMS);
      fprintf(fp,"BIASS=%i\n",BIASS);
      fprintf(fp,"SHIFTS=%i\n",SHIFTS);*/
  fprintf(fp,"red_impact_par_mantissa_bits=%i\n",23-SHIFTS);

  /*  fprintf(fp,"MIND=%g\n",MIND);
      fprintf(fp,"MAXD=%g\n",MAXD);
      fprintf(fp,"DIMD=%i\n",DIMD);
      fprintf(fp,"BIASD=%i\n",BIASD);
      fprintf(fp,"SHIFTD=%i\n",SHIFTD);*/
  fprintf(fp,"e_stop_mantissa_bits=%i\n",23-SHIFTD);
  return 0;
}

int print_some_simulation_parameters(FILE* fp,int ion_number){
  /* print some information on the current simulation to the stream pointed to by fp */
  time_t current;
  char timestr[20];
  current=time(NULL);
  fprintf(fp,"[Simulation]\n");
  fprintf(fp,"ions_simulated=%i\n",ion_number);
  fprintf(fp,"cmd_line_override_number_of_ions=%i\n",override_max_ions);
  fprintf(fp,"cmd_line_override_ion_energy=%g\n",override_energy);
  fprintf(fp,"ion_energy=%g\n",ionInitialEnergy);
  strftime(timestr,20,"%Y-%m-%d_%H:%M:%S",localtime(&sim_start_time));
  fprintf(fp,"simulation_start=%s\n",timestr);
  strftime(timestr,20,"%Y-%m-%d_%H:%M:%S",localtime(&current));
  fprintf(fp,"results_stored__=%s\n\n",timestr);
  return 0;
}

int print_stopping_table(int ionZ, double ionM, int target, double e_min, double e_max, double e_step){
  /* prints value for electronic stopping of one element in a defined target */
  double stopping, energy;
  energy=e_min;

  printf("#Printing stopping table for ion: %i, mass: %f, target material: %s\n",ionZ,ionM,ListOfMaterials[target].Name);
  printf("#Energy/eV\tStopping/(eV/nm)\n");

  while(energy<=e_max){


    stopping   = 10.0*ElectronicStopping(ionZ,ionM,energy,target);
    /* factor 10, because flightpath is in nm, stopping in A */

    printf("%g\t%g\n",energy,stopping);

    energy+=e_step;
  }
  printf("#Finished.\n");
  return 0;
}

/* CROC Modified Kinchin Pease damage see page 7-28 of SRIM book ZBZ*/
int prepare_KP_tables2 (void) {
  int i, k;

  for (i=0; i<NumberOfMaterials; i++) {
    ListOfMaterials[i].MeanEd=0;
    for(k=0;k<ListOfMaterials[i].ElementCount;k++){    
      ListOfMaterials[i].MeanEd=ListOfMaterials[i].MeanEd+ListOfMaterials[i].ElementsConc[k]*ListOfMaterials[i].ElementsDispEnergy[k];
    }
    /*   SRIM like*/
    ListOfMaterials[i].k_d= 0.1334 * pow ( ListOfMaterials[i].MeanZ, 2.0 / 3.0) / pow ( ListOfMaterials[i].MeanM, 0.5);
    ListOfMaterials[i].ed_oE=0.01014 * pow (ListOfMaterials[i].MeanZ , -7.0 / 3.0) ; 
    /*  MyTrim like*/
    /*      ListOfMaterials[i].k_d= 0.1337 * pow ( ListOfMaterials[i].MeanZ, 2.0 / 3.0) / pow ( ListOfMaterials[i].MeanM, 0.5);
	    ListOfMaterials[i].ed_oE=0.0115 * pow (ListOfMaterials[i].MeanZ , -7.0 / 3.0) ; 
    */
  }

  return 0;
}

void message(int level, char* msg, ...){
  /* print out message to console. */
  /* caller must make sure that output text is shorter than 4095 characters... otherwise crash possible. No boundary check to save time */
  if(level<=print_level){
    va_list args;
    va_start(args, msg);
    vsnprintf(message_buffer, 4095, msg, args);
    printf(message_buffer);
    va_end(args);
  }
}
void message_error(int err_number, char* msg, ...){
  /* print out message to stderr. */
  /* caller must make sure that output text is shorter than 4080 characters... otherwise crash possible. No boundary check to save time */
  va_list args;
  va_start(args, msg);
  sprintf(message_buffer,"ERROR (%05i): ",err_number);
  vsnprintf(message_buffer+15, 4080, msg, args);
  fprintf(stderr,message_buffer);
  va_end(args);
}
void message_debug(char* file, int line, char* msg, ...){
  /* print out debug message to stdout. */
  /* Debug message is only printed if ion_number >= MONITOR_ION or if ion_number is 0) */
  /* Note: this function should ONLY be called within #ifdef DEBUG_MODE structures */
  if((ion_c==0)||(ion_c>=MONITOR_ION)){
    int start;
    va_list args;
    va_start(args, msg);
    sprintf(message_buffer,"DEBUG %s line %i, ion %i: ",file,line,ion_c);
    start=strlen(message_buffer);
    vsnprintf(message_buffer+start, 4000, msg, args);
    fprintf(stderr,message_buffer);
    fflush(stderr);
    va_end(args);
  }
}

int DensityScaleArray( int* unscl, float* scl, float conc) {
  /* Added by JP Croc for output in DPA */
  int i,current_material_index ;
  float factor;
  for(i=0;i<cell_count;i++){ /* loop cells */
    current_material_index=TargetComposition[i];
    factor = unit_conversion_factor/(ListOfMaterials[current_material_index].Density*conc);
    if(!isfinite(factor)){factor=0;} /* avoid undefined values for zero density in vacuum */
    scl[i]=unscl[i]*factor;     /* Sum of vacancies */
  }
  return 0;
}

  
