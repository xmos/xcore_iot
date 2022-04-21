@Library('xmos_jenkins_shared_library@v0.18.0') _

// wait here until specified artifacts appear
def artifactUrls = getGithubArtifactUrls([
    "xcore_sdk_bare-metal_example_apps"
    // "xcore_sdk_freertos_example_apps",
    // "xcore_sdk_freertos_usb_example_apps",
    // "xcore_sdk_host_apps",
    // "xcore_sdk_reference_apps",
    // "xcore_sdk_rtos_tests"
])

def distDir = "dist"

getApproval()

pipeline {
    agent {
        label 'sdk'
    }
    options {
        skipDefaultCheckout()
    }    
    stages {
        stage('Download artifacts') {
            steps {
                dir("${distDir}") {
                    downloadExtractZips(artifactUrls)
                    sh "ls -la"
                }
            }
        }
        // stage('Reset XTAGs'){
        //     steps{
        //         dir("${distDir}") {
        //             sh "rm -f ~/.xtag/acquired" //Hacky but ensure it always works even when previous failed run left lock file present
        //             withVenv {
        //                 sh "pip install git+https://github0.xmos.com/xmos-int/xtagctl.git"
        //                 sh "xtagctl reset_all XCORE-AI-EXPLORER"
        //             }
        //         }
        //     }
        // }        
        stage('Run bare-metal examples') {
            steps {
                dir("${distDir}") {
                    sh "xrun --xscope example_bare_metal_vww.xe"
                }
            }
        }
    }
    post {
        cleanup {
            cleanWs()
        }
    }
}