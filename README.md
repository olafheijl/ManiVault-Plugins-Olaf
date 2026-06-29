# ManiVault Plugins for HSNE-based Multivariate Volume Rendering
This repository contains the ManiVault plugins developed and modified for the master thesis:

Hierarchical Dimensionality Reduction for Scalable Multivariate Volume Rendering
Olaf Heijl, Delft University of Technology
https://repository.tudelft.nl/record/uuid:977ed9b3-5587-4c81-82a3-a0ec9d352244

The project explores how Hierarchical Stochastic Neighbor Embedding (HSNE) can be used as a scalable alternative to flat dimensionality reduction methods, such as t-SNE, for transfer function design in multivariate direct volume rendering.

Instead of defining a transfer function over all voxels in a large flat embedding, this implementation allows transfer functions to be defined over a selected HSNE landmark level. This reduces visual clutter in the embedding space and decreases the number of points involved in nearest-neighbor based transfer function evaluation.

# Requirements

This project is intended to be built as part of a ManiVault development setup.

Required:

- ManiVault Studio
- CMake 3.22 or newer
- Qt 6
- A C++20-compatible compiler
- OpenGL-capable GPU for the rendering components

On Windows, Visual Studio 2022 is recommended.

Depending on the exact setup, the t-SNE/HSNE analysis plugin and HDILib dependencies may also be required.

# Build Instructions
Use the main branch. The other branch is used for experiments in the thesis. 
- First install and build ManiVault Studio (https://github.com/ManiVaultStudio).
- Then clone this repository.

# Code Overview
├── DVRTransferFunctionPlugin/     # Transfer function editor for embedding space 
├── DVRViewPlugin/                 # Direct volume rendering view plugin 
├── DVRVolumeLoaderPlugin/         # Volume data loading functionality 
├── VolumeDataPlugin/              # Volume data representation used by the plugins
