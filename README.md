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

Use the `main` branch for the stable version of the plugins. The other branch contains experimental code used during the thesis.

1. Install and build ManiVault Studio: https://github.com/ManiVaultStudio
2. Clone this repository.
3. Build the plugins as part of a ManiVault development setup.

# Code Overview

The repository contains four plugin folders:

* `DVRTransferFunctionPlugin`: Transfer function editor for the embedding space. This plugin provides the transfer function information used by the DVR view plugin.
* `DVRViewPlugin`: Direct volume rendering view plugin. This plugin renders the volume using transfer function data from the transfer function plugin.
* `DVRVolumeLoaderPlugin`: Volume data loading functionality.
* `VolumeDataPlugin`: Volume data representation used by the other plugins.

These plugins are based on existing ManiVault plugin implementations and were adapted and extended for the thesis project. The main thesis-specific changes focus on HSNE-based transfer function design and the rendering pipelines used for multivariate volume rendering.

# Acknowledgements / Attribution

This work builds on the ManiVault framework and existing ManiVault plugin implementations.

The plugins in this repository were adapted from earlier ManiVault volume rendering and transfer function plugins developed by Ravi Snellenberg at https://github.com/Rsnelllenberg/VolumeProjectorPlugin. The original plugins were modified and extended for the master thesis *Hierarchical Dimensionality Reduction for Scalable Multivariate Volume Rendering*.

The thesis-specific contributions include the integration of HSNE landmark embeddings for transfer function design, changes to the transfer function workflow, and adaptations to the direct volume rendering pipeline for evaluating HSNE-based and t-SNE-based approaches.

# License

This project is licensed under the GPL-3.0 license. See `LICENSE` for details.

Parts of the code are derived from earlier ManiVault plugin implementations. Please also check the license and attribution requirements of the original repository.
