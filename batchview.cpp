#include "batchview.h"
#include "ui_batchview.h"

BatchView::BatchView(QWidget *parent, PanoramaViewer* pano, int batchmode, int batchviewmode) :
    QMainWindow(parent),
    ui(new Ui::BatchView)
{
    ui->setupUi(this);

    this->pano = pano;

    this->MainLayout = new FlowLayout();

    // Center window on screen
    this->setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            this->size(),
            qApp->desktop()->availableGeometry()
        ));

    // Start window maximized
    this->showMaximized();

    this->MainLayout->setContentsMargins( 10, 10, 10, 10 ) ;

    this->setMode(batchmode);
    this->populate(batchviewmode);

    QWidget * dummy = new QWidget();
    dummy->setLayout(MainLayout);

    this->ui->scrollArea->setWidget(dummy);

    // Keyboard shortcuts
    new QShortcut(QKeySequence("Ctrl+a"), this, SLOT(selectAll()));
    new QShortcut(QKeySequence("Ctrl+d"), this, SLOT(unSelectAll()));
    new QShortcut(QKeySequence("Ctrl+i"), this, SLOT(invertSelection()));

    QStandardItemModel* model =
        qobject_cast<QStandardItemModel*>(this->ui->TypeList->model());
    QModelIndex firstIndex = model->index(0, this->ui->TypeList->modelColumn(),
                                          this->ui->TypeList->rootModelIndex());
    QStandardItem* firstItem = model->itemFromIndex(firstIndex);
    firstItem->setSelectable(false);

    this->pressed_keys.CTRL = false;

    // Connect signal for labels refresh
    connect(this, SIGNAL(refreshLabels()), parent, SLOT(refreshLabels()));
}

void BatchView::populate(int batchviewmode)
{

    foreach(ObjectRect* rect, pano->rect_list )
    {
        switch(batchviewmode)
        {
        case BatchViewMode::All:
            this->insertItem(rect);
            break;
        case BatchViewMode::OnlyUntyped:
            if(rect->getType() == ObjectType::None)
            {
                this->insertItem(rect);
            }
            break;
        case BatchViewMode::OnlyFaces:
            if(rect->getType() == ObjectType::Face &&
                    (rect->getAutomaticStatus() == "Valid" || rect->getAutomaticStatus() == "None"))
            {
                this->insertItem(rect);
            }
            break;
        case BatchViewMode::OnlyUnapprovedFaces:
            if(rect->getType() == ObjectType::Face
                    && rect->getManualStatus() == "None"
                    && rect->getAutomaticStatus() == "Valid")
            {
                this->insertItem(rect);
            }
            break;
        case BatchViewMode::OnlyNumberPlates:
            if(rect->getType() == ObjectType::NumberPlate)
            {
                this->insertItem(rect);
            }
            break;
        case BatchViewMode::OnlyUnapprovedNumberPlates:
            if(rect->getType() == ObjectType::NumberPlate
                    && rect->getManualStatus() == "None"
                    && rect->getAutomaticStatus() == "Valid")
            {
                this->insertItem(rect);
            }
            break;
        case BatchViewMode::OnlyPreInvalidated:
            if(rect->getType() != ObjectType::None
                    && rect->getAutomaticStatus() != "Valid"
                    && rect->getAutomaticStatus() != "None")
            {
                this->insertItem(rect);
            }
            break;
        case BatchViewMode::OnlyToBlur:
            if(rect->getType() == ObjectType::ToBlur)
            {
                this->insertItem(rect);
            }
            break;
        }
    }

}

void BatchView::insertItem(ObjectRect *rect)
{
    ObjectItem* object = new ObjectItem(0, this->pano, rect);

    object->setSize( QSize(this->ui->horizontalSlider->value(), this->ui->horizontalSlider->value()) );

    this->MainLayout->addWidget(object);
    this->elements.append(object);
}

BatchView::~BatchView()
{
    delete ui;
}

void BatchView::setMode(int mode)
{   
    switch(mode)
    {
    case BatchMode::Manual:
        this->ui->InvalidateButton->setEnabled(false);
        this->ui->ValidateButton->setEnabled(false);
        break;
    case BatchMode::Auto:
        this->ui->setType->setEnabled(false);
        this->ui->TypeList->setEnabled(false);
        this->ui->setSubType->setEnabled(false);
        this->ui->SubTypeList->setEnabled(false);
        this->ui->deleteButton->setEnabled(false);
        break;
    case BatchMode::ToBlur:
        this->ui->setType->setEnabled(false);
        this->ui->TypeList->setEnabled(false);
        this->ui->InvalidateButton->setEnabled(false);
        this->ui->ValidateButton->setEnabled(false);
        this->ui->BlurButton->setEnabled(false);
        this->ui->NoBlurButton->setEnabled(false);
        break;
    }
}

void BatchView::selectAll()
{
    foreach(ObjectItem* item, this->elements )
    {
        item->setSelected(true);
    }
}

void BatchView::unSelectAll()
{
    foreach(ObjectItem* item, this->elements )
    {
        item->setSelected(false);
    }
}

void BatchView::invertSelection()
{
    foreach(ObjectItem* item, this->elements )
    {
        item->setSelected( !item->selected );
    }
}

void BatchView::on_horizontalSlider_sliderMoved(int position)
{
    foreach(ObjectItem* item, this->elements )
    {
        item->setSize(QSize(position, position));
    }
}

void BatchView::on_CancelButton_clicked()
{
    this->close();
}

void BatchView::on_NoBlurButton_clicked()
{
    foreach(ObjectItem* item, this->elements )
    {
        if(item->selected)
        {
            item->setBlurred(false);
        }
    }
}

void BatchView::on_deleteButton_clicked()
{
    foreach(ObjectItem* item, this->elements )
    {
        if(item->selected)
        {
            item->remove( true );
        }
    }
}

void BatchView::on_BlurButton_clicked()
{
    foreach(ObjectItem* item, this->elements )
    {
        if(item->selected)
        {
            item->setBlurred(true);
        }
    }
}

void BatchView::on_ValidateButton_clicked()
{
    foreach(ObjectItem* item, this->elements )
    {
        if(item->selected)
        {
            item->setValidState(ObjectValidState::Valid);
            item->setManualStatus("Valid");
        }
    }
}

void BatchView::on_InvalidateButton_clicked()
{
    foreach(ObjectItem* item, this->elements )
    {
        if(item->selected)
        {
            item->setValidState(ObjectValidState::Invalid);
            item->setManualStatus("Invalid");
        }
    }
}

void BatchView::on_setType_clicked()
{
    foreach(ObjectItem* item, this->elements )
    {
        if(item->selected)
        {
            item->setType(this->ui->TypeList->currentIndex());
        }
    }
}

void BatchView::on_setSubType_clicked()
{
    foreach(ObjectItem* item, this->elements )
    {
        if(item->selected)
        {
            item->setSubType(this->ui->SubTypeList->currentIndex());
        }
    }
}

void BatchView::mergeResults()
{
    // Merge existing objects
    foreach(ObjectItem* item, this->elements )
    {
        foreach (ObjectRect* rect, this->pano->rect_list) {
            if(rect->getId() == item->id)
            {
                rect->mergeWith( item->rect );
            }
        }
    }

    // Delete requested objects
    foreach(ObjectItem* item, this->elements )
    {
        foreach (ObjectRect* rect, this->pano->rect_list) {
            if(item->toBeRemoved && (item->id == rect->getId()))
            {
                this->pano->rect_list.removeOne( rect );
                delete rect;
            }
        }
    }
}

void BatchView::on_ApplyButton_clicked()
{
    bool replyMSG = true;
    bool haveNoClass = false;
    bool haveNoManualState = false;

    foreach(ObjectItem* item, this->elements)
    {
        if(item->type == ObjectType::None)
        {
            haveNoClass = true;
            break;
        }
    }

    foreach(ObjectItem* item, this->elements)
    {
        if(item->manualStatus == "None")
        {
            haveNoManualState = true;
            break;
        }
    }

    if(haveNoClass)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Warning", "Not all objects have a class defined, quit anyway?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No) {
            replyMSG = false;
        }

        if(replyMSG)
        {
            this->mergeResults();
            emit refreshLabels();
            this->pano->render();
            this->close();
        }
    } else if(!haveNoClass && haveNoManualState) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Warning", "Not all objects have been validated, quit anyway?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No) {
            replyMSG = false;
        }

        if(replyMSG)
        {
            this->mergeResults();
            emit refreshLabels();
            this->pano->render();
            this->close();
        }
    } else {
        this->mergeResults();
        emit refreshLabels();
        this->pano->render();
        this->close();
    }

}

void BatchView::on_selectAllButton_clicked()
{
    this->selectAll();
}

void BatchView::on_unSelectAllButton_clicked()
{
    this->unSelectAll();
}

void BatchView::on_invertSelectionButton_clicked()
{
    this->invertSelection();
}

void BatchView::wheelEvent(QWheelEvent * event)
{
    int delta = (event->delta() / 120) * 10;

    if( this->pressed_keys.CTRL )
    {
        int newvalue = (this->ui->horizontalSlider->value() + delta);

        if (newvalue < this->ui->horizontalSlider->minimum()
                || newvalue > this->ui->horizontalSlider->maximum())
        {
            return;
        }

        this->ui->horizontalSlider->setValue( newvalue );

        foreach(ObjectItem* item, this->elements )
        {
            item->setSize(QSize(newvalue, newvalue));
        }
    }
}

void BatchView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control)
        this->pressed_keys.CTRL = true;
}

void BatchView::keyReleaseEvent(QKeyEvent *)
{
    this->pressed_keys.CTRL = false;
}
