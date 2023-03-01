# Name:        process_data.py
# Author:      David Borland, The Renaissance Computing Institute (RENCI)
#
# Copyright:   The Renaissance Computing Institute (RENCI)
#
# License:     Licensed under the RENCI Open Source Software License v. 1.0
#
#              See included License.txt or
#              http://www.renci.org/resources/open-source-software-license
#              for details.
#
# Description: Process data for the urban wind analysis (UWA) project


from uwa import *


# Variables
ensight_input_name = "C:/borland/data/Manhattan/FullWindDirections/BC1_ensight/full_trans_125.ensight.encas"
normalized_velocity_output_name = "C:/borland/src/ManhattanProject/Data/Normalized.vtu"
scale_factor = 2.0

stl_input_name = "C:/borland/src/ManhattanProject/Data/Manhattan_STL2.stl"
roof_offset_output_name = "C:/borland/src/ManhattanProject/Data/RoofOffset.vtp"
offset = 10.0

subset_output_name = "C:/borland/src/ManhattanProject/Data/Subset.vtu"
center = [0.0, 0.0, 250.0]
size = [1000.0, 1000.0, 500.0]
rotation = 28.5

rescaled_subset_output_name = "C:/borland/src/ManhattanProject/Data/SubsetRescaled.vtu"
rescale_factor = 0.75

rescaled_roof_offset_output_name = "C:/borland/src/ManhattanProject/Data/RoofOffsetRescaled.vtp"


# Process the EnSight file
process_ensight(ensight_input_name, normalized_velocity_output_name, scale_factor)

# Sample the wind vector field at the given offset
roof_offset(normalized_velocity_output_name, stl_input_name, roof_offset_output_name, offset)

# Extract a subset of the velocity field
extract_subset(normalized_velocity_output_name, subset_output_name, center, size, rotation)

# Rescale the extracted mesh subset velocity field
scale_velocity_mesh(subset_output_name, rescaled_subset_output_name, rescale_factor)

# Rescale the extracted roof offset velocity field
scale_velocity_roof_offset(roof_offset_output_name, rescaled_roof_offset_output_name, rescale_factor)

