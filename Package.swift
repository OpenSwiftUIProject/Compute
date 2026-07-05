// swift-tools-version: 6.2

import PackageDescription

let swiftRuntimeHeadersPath = "\(Context.packageDirectory)/Submodules/swift-runtime-headers"

let package = Package(
    name: "Compute",
    platforms: [.macOS(.v15)],
    products: [
        .library(name: "Compute", targets: ["Compute"]),
        .library(name: "_ComputeTestSupport", targets: ["_ComputeTestSupport"]),
    ],
    traits: [
        .trait(name: "CompatibilityModeAttributeGraphV6")
    ],
    dependencies: [],
    targets: [
        .target(
            name: "Platform",
            cSettings: [
                .define("_GNU_SOURCE", .when(platforms: [.linux]))
            ]
        ),
        .target(
            name: "SwiftCorelibsCoreFoundation"
        ),
        .target(
            name: "Utilities",
            dependencies: [
                "Platform",
                .target(name: "SwiftCorelibsCoreFoundation", condition: .when(platforms: [.linux])),
            ]
        ),
        .target(
            name: "Compute",
            dependencies: ["ComputeCxx"],
            swiftSettings: [
                .enableExperimentalFeature("Extern"),
                .unsafeFlags([
                    "-enable-library-evolution",
                    // When -enable-library-evolution is specified verify-emitted-module-interface command fails
                    "-no-verify-emitted-module-interface",
                ]),
            ]
        ),
        .target(
            name: "ComputeCxx",
            dependencies: [
                "Platform",
                "Utilities",
                "ComputeCxxSwiftSupport",
                .target(name: "SwiftCorelibsCoreFoundation", condition: .when(platforms: [.linux])),
            ],
            cxxSettings: [
                .headerSearchPath(""),
                .headerSearchPath("internalInclude"),
                .define("_GNU_SOURCE", .when(platforms: [.linux])),
                .unsafeFlags([
                    "-static",
                    "-DCOMPILED_WITH_SWIFT",
                    "-DPURE_BRIDGING_MODE",
                    "-isystem", "\(swiftRuntimeHeadersPath)/include",
                    "-isystem", "\(swiftRuntimeHeadersPath)/stdlib/include",
                    "-isystem", "\(swiftRuntimeHeadersPath)/stdlib/public/SwiftShims",
                ]),
            ]
        ),
        .target(name: "ComputeCxxSwiftSupport"),
        .target(name: "_ComputeTestSupport"),
    ],
    cxxLanguageStandard: .cxx20,
)
