
#   Generate some output
# @TEST-EXEC: bro -r %DIR/../traces/workshop.trace1.trace %INPUT tcp


#   Verify the log file, and stderr/out match the Baseline
# @TEST-EXEC: btest-diff KNOWN_SERVICES
# @TEST-EXEC: btest-diff .stderr
# @TEST-EXEC: btest-diff .stdout


# Load the script we're here to test
@load known-services

# Make some changes to how it runs
export {
        # Log everything, so we get some output
        redef KnownServices::logged_hosts=Enabled;
}

# If necessary, can take setup action here as well
event bro_init()
{
}
