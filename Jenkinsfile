@Library('xmos_jenkins_shared_library@v0.14.2') _

getApproval()

pipeline {
    agent {
        dockerfile {
        }
    }

    parameters { // Available to modify on the job page within Jenkins if starting a build
        string( // use to try different tools versions
            name: 'TOOLS_VERSION',
            defaultValue: '15.0.1',
            description: 'The tools version to build with (check /projects/tools/ReleasesTools/)'
        )
        booleanParam( // use to check results of rolling all conda deps forward
            name: 'UPDATE_ALL',
            defaultValue: false,
            description: 'Update all conda packages before building'
        )
    }

    options { // plenty of things could go here
        //buildDiscarder(logRotator(numToKeepStr: '10'))
        timestamps()
    }

    stages {
        stage("Setup") {
            // Clone and install build dependencies
            steps {
                // clean auto default checkout
                sh "rm -rf *"
                // clone
                checkout([
                    $class: 'GitSCM',
                    branches: scm.branches,
                    doGenerateSubmoduleConfigurations: false,
                    extensions: [[$class: 'SubmoduleOption',
                                  threads: 8,
                                  timeout: 20,
                                  shallow: true,
                                  parentCredentials: true,
                                  recursiveSubmodules: true],
                                 [$class: 'CleanCheckout']],
                    userRemoteConfigs: [[credentialsId: 'xmos-bot',
                                         url: 'git@github.com:xmos/ai_deployment_framework']]
                ])
                // create venv
                sh "conda env create -q -p adf_venv -f environment.yml"
                // Install xmos tools version
                sh "/XMOS/get_tools.py " + params.TOOLS_VERSION
            }
        }
        stage("Update all packages") {
            // Roll all conda packages forward beyond their pinned versions
            when { expression { return params.UPDATE_ALL } }
            steps {
                sh "conda update --all -y -q -p adf_venv"
            }
        }
        stage("Build/Test") {
            // due to the Makefile, we've combined build and test stages
            steps {
                // below is how we can activate the tools
                sh """pushd /XMOS/tools/${params.TOOLS_VERSION}/XMOS/xTIMEcomposer/${params.TOOLS_VERSION} && . SetEnv && popd &&
                      . activate ./adf_venv &&
                      make ci"""
                // Any call to pytest can be given the "--junitxml SOMETHING_junit.xml" option
                // This step collects these files for display in Jenkins UI
                junit "**/*_junit.xml"
            }
        }
    }
    post {
        cleanup {
            cleanWs()
        }
    }
}
