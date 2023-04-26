use std::collections::HashMap;

use goblin::mach::{load_command::CommandVariant, symbols::Nlist64};

#[macro_use]
extern crate log;

fn copy_dir_all(src: impl AsRef<std::path::Path>, dst: impl AsRef<std::path::Path>) {
    std::fs::create_dir_all(&dst).unwrap();
    for entry in std::fs::read_dir(src).unwrap().flatten() {
        if entry.file_type().unwrap().is_dir() {
            copy_dir_all(entry.path(), dst.as_ref().join(entry.file_name()));
        } else {
            std::fs::copy(entry.path(), dst.as_ref().join(entry.file_name())).unwrap();
        }
    }
}

fn main() {
    simple_logger::init().unwrap();

    let Some(kext_path) = std::env::args().nth(1).map(std::path::PathBuf::from) else {
        error!("Usage: {} <kext_path>", std::env::args().next().unwrap());
        return;
    };
    let kext_name = kext_path.file_stem().unwrap();
    let exec_data =
        std::fs::read(kext_path.join("Contents").join("MacOS").join(kext_name)).unwrap();
    let obj = goblin::Object::parse(&exec_data).unwrap();
    let goblin::Object::Mach(goblin::mach::Mach::Binary(macho)) = obj else {
        panic!("not a Mach-O binary");
    };
    assert!(macho.is_64);
    let (cmd_off, nsyms, symoff) = macho
        .load_commands
        .iter()
        .find_map(|cmd| {
            let CommandVariant::Symtab(v) = cmd.command else {
                return None;
            };
            Some((cmd.offset, v.nsyms as usize, v.symoff as usize))
        })
        .unwrap();

    let orig2fixed = HashMap::from([
        ("__ZN14IOAccelShared217lookupDeviceShmemEj", "__ZNK14IOAccelShared217lookupDeviceShmemEj"),
        ("__ZN15IOAccelContext215contextModeBitsEv", "__ZNK15IOAccelContext215contextModeBitsEv"),
        ("__ZN15IOAccelContext216isTripleBufferedEv", "__ZNK15IOAccelContext216isTripleBufferedEv"),
        ("__ZN15IOAccelContext217getSurfaceReqBitsEv", "__ZNK15IOAccelContext217getSurfaceReqBitsEv"),
        ("__ZN15IOAccelContext219allowsExclusiveModeEv", "__ZNK15IOAccelContext219allowsExclusiveModeEv"),
        ("__ZN15IOAccelContext220requiresBackingStoreEv", "__ZNK15IOAccelContext220requiresBackingStoreEv"),
        ("__ZN15IOAccelSurface216getSurfaceBufferE20eIOAccelGLBufferType", "__ZNK15IOAccelSurface216getSurfaceBufferE20eIOAccelGLBufferType"),
        ("__ZN15IOAccelSurface227convertGLIndexToBufferIndexE20eIOAccelGLBufferType", "__ZNK15IOAccelSurface227convertGLIndexToBufferIndexE20eIOAccelGLBufferType"),
        ("__ZN16IOAccelResource213requirePageonEv", "__ZNK16IOAccelResource213requirePageonEv"),
        ("__ZN16IOAccelResource222newResourceWithOptionsEP22IOGraphicsAccelerator2P14IOAccelShared215eIOAccelResTypeyjyPy", "__ZN16IOAccelResource222newResourceWithOptionsEP22IOGraphicsAccelerator2P14IOAccelShared215eIOAccelResTypeyjyPyj"),
        ("__ZN16IOAccelResource224getClientSharedROPrivateEv", "__ZNK16IOAccelResource224getClientSharedROPrivateEv"),
        ("__ZN16IOAccelResource224getClientSharedRWPrivateEv", "__ZNK16IOAccelResource224getClientSharedRWPrivateEv"),
        ("__ZN16IOAccelSysMemory12getDirtySizeEv", "__ZNK16IOAccelSysMemory12getDirtySizeEv"),
        ("__ZN16IOAccelSysMemory15getResidentSizeEv", "__ZNK16IOAccelSysMemory15getResidentSizeEv"),
        ("__ZN16IOAccelVidMemory12getDirtySizeEv", "__ZNK16IOAccelVidMemory12getDirtySizeEv"),
        ("__ZN16IOAccelVidMemory15getResidentSizeEv", "__ZNK16IOAccelVidMemory15getResidentSizeEv"),
        ("__ZN17IOAccel2DContext217getSurfaceReqBitsEv", "__ZNK17IOAccel2DContext217getSurfaceReqBitsEv"),
        ("__ZN17IOAccel2DContext219allowsExclusiveModeEv", "__ZNK17IOAccel2DContext219allowsExclusiveModeEv"),
        ("__ZN17IOAccelGLContext215contextModeBitsEv", "__ZNK17IOAccelGLContext215contextModeBitsEv"),
        ("__ZN17IOAccelGLContext216isTripleBufferedEv", "__ZNK17IOAccelGLContext216isTripleBufferedEv"),
        ("__ZN17IOAccelGLContext217getSurfaceReqBitsEv", "__ZNK17IOAccelGLContext217getSurfaceReqBitsEv"),
        ("__ZN17IOAccelGLContext219allowsExclusiveModeEv", "__ZNK17IOAccelGLContext219allowsExclusiveModeEv"),
        ("__ZN17IOAccelGLContext220requiresBackingStoreEv", "__ZNK17IOAccelGLContext220requiresBackingStoreEv"),
        ("__ZN18IOAccelStatistics227_RESERVEDIOAccelStatistics0Ev", "__ZN18IOAccelStatistics216setupIOReportersEv"),
        ("__ZN18IOAccelStatistics227_RESERVEDIOAccelStatistics1Ev", "__ZN18IOAccelStatistics213setupChannelsEv"),
        ("__ZN18IOAccelStatistics227_RESERVEDIOAccelStatistics2Ev", "__ZN18IOAccelStatistics215configureReportEP19IOReportChannelListjPvS2_"),
        ("__ZN18IOAccelStatistics227_RESERVEDIOAccelStatistics3Ev", "__ZN18IOAccelStatistics212updateReportEP19IOReportChannelListjPvS2_"),
        ("__ZN19IOAccelCommandQueue19retireCommandBufferEP17IOAccelBlockFence", "__ZN19IOAccelCommandQueue19retireCommandBufferEP17IOAccelEventFence"),
        ("__ZN19IOAccelCommandQueue29_RESERVEDIOAccelCommandQueue1Ev", "__ZN19IOAccelCommandQueue21processKernelCommandsEPK20IOAccelKernelCommandS2_"),
        ("__ZN19IOAccelCommandQueue29_RESERVEDIOAccelCommandQueue2Ev", "__ZN19IOAccelCommandQueue20processKernelCommandEPK20IOAccelKernelCommandS2_"),
        ("__ZN19IOAccelDisplayPipe214getFramebufferEv", "__ZNK19IOAccelDisplayPipe214getFramebufferEv"),
        ("__ZN22IOAccelDisplayMachine214getDisplayPipeEj", "__ZNK22IOAccelDisplayMachine214getDisplayPipeEj"),
        ("__ZN22IOAccelDisplayMachine216getIOFramebufferEj", "__ZNK22IOAccelDisplayMachine216getIOFramebufferEj"),
        ("__ZN22IOAccelDisplayMachine217getScanoutSurfaceEj", "__ZNK22IOAccelDisplayMachine217getScanoutSurfaceEj"),
        ("__ZN22IOAccelDisplayMachine218getScanoutResourceEjj", "__ZNK19IOAccelDisplayPipe218getScanoutResourceEj"),
        ("__ZN22IOAccelDisplayMachine219getFramebufferCountEv", "__ZNK22IOAccelDisplayMachine219getFramebufferCountEv"),
        ("__ZN22IOAccelDisplayMachine219isFramebufferActiveEj", "__ZNK22IOAccelDisplayMachine219isFramebufferActiveEj"),
        ("__ZN22IOAccelDisplayMachine220getFullScreenSurfaceEj", "__ZNK22IOAccelDisplayMachine220getFullScreenSurfaceEj"),
        ("__ZN22IOAccelDisplayMachine222getFramebufferResourceEjj", "__ZNK22IOAccelDisplayMachine222getFramebufferResourceEjj"),
        ("__ZN22IOGraphicsAccelerator231_RESERVEDIOGraphicsAccelerator0Ev", "__ZN22IOGraphicsAccelerator218getMaxResourceSizeEv"),
    ]);

    let mut new_str_table = vec![0u8; 4];
    let mut off2fixed = HashMap::new();
    for (sym, off) in macho
        .symbols()
        .flatten()
        .map(|(sym, nlist)| (sym, nlist.n_strx))
    {
        off2fixed.insert(off, new_str_table.len());
        let v = *orig2fixed.get(sym).unwrap_or(&sym);
        if sym != v {
            info!("{sym} -> {v}");
        }
        new_str_table.extend_from_slice(v.as_bytes());
        new_str_table.push(0);
    }
    new_str_table.extend_from_slice(&[0u8; 4]);

    let mut new_data = exec_data.clone();
    new_data[cmd_off + 16..cmd_off + 20].copy_from_slice(&(exec_data.len() as u32).to_le_bytes()); // stroff
    new_data[cmd_off + 20..cmd_off + 24]
        .copy_from_slice(&(new_str_table.len() as u32).to_le_bytes()); // strsize
    new_data.extend_from_slice(&new_str_table);
    for i in 0..nsyms {
        let nlist_off = symoff + i * std::mem::size_of::<Nlist64>();
        let u_un = u32::from_le_bytes(new_data[nlist_off..nlist_off + 4].try_into().unwrap());
        new_data[nlist_off..nlist_off + 4]
            .copy_from_slice(&(*off2fixed.get(&(u_un as usize)).unwrap() as u32).to_le_bytes())
    }
    let result_path = kext_path.join("..").join("Results");
    copy_dir_all(
        kext_path.clone(),
        result_path.join(kext_path.file_name().unwrap()),
    );
    std::fs::write(
        result_path
            .join(kext_path.file_name().unwrap())
            .join("Contents")
            .join("MacOS")
            .join(kext_name),
        new_data,
    )
    .unwrap();
    info!("Done");
}
