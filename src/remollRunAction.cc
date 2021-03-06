#include "remollRunAction.hh"

#include "G4RunManager.hh"

#include "remollIO.hh"
#include "remollRun.hh"
#include "remollRunData.hh"
#include "remollBeamTarget.hh"

#include "G4Threading.hh"
#include "G4AutoLock.hh"
namespace { G4Mutex remollRunActionMutex = G4MUTEX_INITIALIZER; }

remollRunAction::remollRunAction() { }

remollRunAction::~remollRunAction() { }

G4Run* remollRunAction::GenerateRun()
{
  return new remollRun();
}

void remollRunAction::BeginOfRunAction(const G4Run* run)
{
  // Cast into remollRun
  const remollRun* aRun = static_cast<const remollRun*>(run);

  // Print progress
  G4int interval = 100; // Print this many progress points (i.e. 100 -> every 1%)
  G4int evts_to_process = aRun->GetNumberOfEventToBeProcessed();
  G4RunManager::GetRunManager()->SetPrintProgress((evts_to_process > interval)
                                                  ? evts_to_process/interval
                                                  : 1);

  if (IsMaster())
  {
    G4cout << "### Run " << aRun->GetRunID() << " start." << G4endl;

    G4AutoLock lock(&remollRunActionMutex);
    remollIO* io = remollIO::GetInstance();
    io->InitializeTree();

    remollRunData *rundata = remollRun::GetRunData();
    rundata->SetNthrown( aRun->GetNumberOfEventToBeProcessed() );
    rundata->Print();
  }
}

void remollRunAction::EndOfRunAction(const G4Run* run)
{
  // Cast into remollRun
  const remollRun* aRun = static_cast<const remollRun*>(run);

  if (IsMaster())
  {
      G4cout << "### Run " << aRun->GetRunID() << " ended." << G4endl;

      G4AutoLock lock(&remollRunActionMutex);
      remollIO* io = remollIO::GetInstance();
      io->WriteTree();
  }
}

